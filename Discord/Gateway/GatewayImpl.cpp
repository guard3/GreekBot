#include "GatewayImpl.h"
#include "Utils.h"
/* ========== Gateway event/command opcodes ========================================================================= */
enum eGatewayOpcode {
	OP_DISPATCH              = 0,  // An event was dispatched.
	OP_HEARTBEAT             = 1,  // Fired periodically by the client to keep the connection alive.
	// Identify              = 2,  // Starts a new session during the initial handshake.
	// Presence update       = 3,  // Update the client's presence.
	// Voice state update    = 4,  // Used to join/leave or move between voice channels.
	// Resume                = 6,  // Resume a previous session that was disconnected.
	OP_RECONNECT             = 7,  // You should attempt to reconnect and resume immediately.
	// Request guild members = 8,  // Request information about offline guild members in a large guild.
	OP_INVALID_SESSION       = 9,  // The session has been invalidated. You should reconnect and identify/resume accordingly.
	OP_HELLO                 = 10, // Sent immediately after connecting, contains the heartbeat_interval to use.
	OP_HEARTBEAT_ACK         = 11, // Sent in response to receiving a heartbeat to acknowledge that it has been received.
};
/* ================================================================================================================== */
cGateway::implementation::implementation(cGateway* p, std::string_view t, eIntent i) :
	m_parent(p),
	m_ctx(asio::ssl::context::tlsv13_client),
	m_ws_strand(asio::make_strand(m_ioc)),
	m_http_strand(asio::make_strand(m_ioc)),
	m_resolver(m_http_strand),
	m_http_auth(fmt::format("Bot {}", t)),
	m_http_timer(m_http_strand),
	m_intents(i),
	m_last_sequence(0),
	m_heartbeat_timer(m_ws_strand),
	m_heartbeat_ack(false),
	m_async_status(ASYNC_CLOSE),
	m_inflate_stream{} {
	/* Set SSL context to verify peers */
	m_ctx.set_default_verify_paths();
	m_ctx.set_verify_mode(asio::ssl::verify_peer);
	/* Initialize zlib inflate stream */
	switch (inflateInit(&m_inflate_stream)) {
		case Z_OK:
			break;
		case Z_MEM_ERROR:
			throw std::bad_alloc();
		default:
			throw std::runtime_error("Could not initialize inflate stream");
	}
}
cGateway::implementation::~implementation() {
	inflateEnd(&m_inflate_stream);
}
/* ================================================================================================================== */
void
cGateway::implementation::send(std::string msg) {
	/* Push the new message at the end of the message queue */
	m_queue.push_back(std::move(msg));
	/* If there's no other message in the queue, start the asynchronous send operation */
	if (m_queue.size() == 1) {
		m_ws->async_write(asio::buffer(m_queue.front()), [this](const beast::error_code &ec, size_t) { on_write(ec); });
		m_async_status |= ASYNC_WRITE;
	}
}
/* ================================================================================================================== */
void
cGateway::implementation::close() {
	/* If we're already closing, don't do anything */
	if (m_async_status & ASYNC_CLOSE)
		return;
	/* Stop heartbeating to prevent writing further data to the stream */
	m_heartbeat_timer.cancel();
	/* Close the stream */
	m_ws->async_close(beast::websocket::close_code::none, [this](auto&&) { on_close(); });
	m_async_status |= ASYNC_CLOSE;
}
void
cGateway::implementation::on_close(bool bRestart) {
	/* Stop heartbeating to prevent writing further data to the stream */
	if (!(m_async_status & ASYNC_CLOSE)) {
		m_heartbeat_timer.cancel();
		m_async_status |= ASYNC_CLOSE;
	}
	struct {
		implementation& self; bool bRestart;
		void operator()() {
			/* Wait for all async operations to finish */
			if (self.m_async_status & (ASYNC_READ | ASYNC_WRITE))
				return asio::defer(self.m_ws_strand, *this);
			/* If we have to exit, simply return and wait for the io context to run out of work */
			if (!bRestart) return;
			/* Reset all resources */
			inflateReset(&self.m_inflate_stream);
			self.m_queue.clear();
			self.m_buffer.clear();
			self.m_ws.reset();
			/* Start a new session */
			self.run_session();
		}
	} _{ *this, bRestart }; _();
}
/* ================================================================================================================== */
void
cGateway::implementation::on_read(const beast::error_code& ec, std::size_t bytes_read) try {
	m_async_status &= ~ASYNC_READ;
	/* If the operation was cancelled, simply return */
	if (m_async_status & ASYNC_CLOSE || ec == asio::error::operation_aborted)
		return;
	/* If the connection was unexpectedly closed... */
	if (ec == asio::error::eof)
		return on_close();
	/* If the session was closed gracefully by the server... */
	if (ec == beast::websocket::error::closed) {
		switch (auto& rsn = m_ws->reason(); rsn.code) {
			case 4004: // Authentication failed
			case 4010: // Invalid shard
			case 4011: // Sharding required
			case 4012: // Invalid API version
			case 4013: // Invalid intent(s)
			case 4014: // Disallowed intent(s)
				/* Cancel the http timer to stop giving work to the io context */
				asio::post(m_http_strand, [this] { m_http_timer.cancel(); });
				/* Report the close reason as the error message */
				cUtils::PrintErr("Fatal gateway error{}{}", rsn.reason.empty() ? "." : ": ", rsn.reason.c_str());
				/* Continue closing without creating a new session */
				return on_close(false);
			case 4007: // Invalid seq
				/* Reset session */
				m_last_sequence = 0;
				m_session_id.clear();
				m_resume_gateway_url.clear();
			default:
				/* Continue closing */
				return on_close();
		}
	}
	/* If any other error occurs, report it and close the connection */
	if (ec) {
		const char* err = ec.message(m_err_msg, std::size(m_err_msg));
		cUtils::PrintErr("An error occurred while reading from the gateway{}{}", *err ? ": " : ".", err);
		return close();
	}
	/* Clear memory and reset the parser prior to parsing a new JSON */
	m_parser.reset();
	/* Check for Z_SYNC_FLUSH suffix and decompress if necessary */
	char* in = (char*)m_buffer.data().data();
	if (bytes_read >= 4 ) {
		if (0 == memcmp(in + bytes_read - 4, "\x00\x00\xFF\xFF", 4)) {
			m_inflate_stream.avail_in = bytes_read;
			m_inflate_stream.next_in  = (Byte*)in;
			do {
				m_inflate_stream.avail_out = std::size(m_inflate_buffer);
				m_inflate_stream.next_out  = m_inflate_buffer;
				switch (inflate(&m_inflate_stream, Z_NO_FLUSH)) {
					case Z_OK:
					case Z_STREAM_END:
						m_parser.write((char*)m_inflate_buffer, std::size(m_inflate_buffer) - m_inflate_stream.avail_out);
						break;
					case Z_MEM_ERROR:
						throw std::bad_alloc();
					default:
						throw std::runtime_error(m_inflate_stream.msg);
				}
			} while (m_inflate_stream.avail_out == 0);
			goto LABEL_PARSED;
		}
	}
	/* If the payload isn't compressed, directly feed it into the json parser */
	m_parser.write(in, bytes_read);
LABEL_PARSED:
	/* Consume all read bytes from the dynamic buffer */
	m_buffer.consume(bytes_read);
	/* Release the parsed json value */
	const json::value v = m_parser.release();
	/* Start the next asynchronous read operation to keep listening for more events */
	m_ws->async_read(m_buffer, [this](const beast::error_code& ec, std::size_t size) { on_read(ec, size); });
	m_async_status |= ASYNC_READ;
	cUtils::PrintDbg("{}", json::serialize(v));
	/* Process the event */
	switch (v.at("op").to_number<int>()) {
		case OP_DISPATCH:
			/* Process event */
			process_event(v);
			break;
		case OP_HEARTBEAT:
			/* Cancel any pending heartbeats */
			if (m_heartbeat_timer.cancel() != 0) {
				/* Send a heartbeat immediately */
				heartbeat();
				/* Resume heartbeating */
				m_heartbeat_timer.expires_after(m_heartbeat_interval);
				m_heartbeat_timer.async_wait([this](beast::error_code ec) { on_expire(ec); });
			}
			/* If cancel() returns 0, then a heartbeat is already queued to be sent, so we don't need to do anything */
			break;
		case OP_INVALID_SESSION:
			/* If the current session can't be resumed, reset it... */
			if (!v.at("d").as_bool()) {
				m_last_sequence = 0;
				m_session_id.clear();
				m_resume_gateway_url.clear();
			}
			close();
		case OP_RECONNECT:
			/* ...and do nothing and wait for the server to close the WebSocket session */
			break;
		case OP_HELLO:
			/* Update heartbeat interval */
			m_heartbeat_ack = true;
			m_heartbeat_interval = chrono::milliseconds(v.at("d").at("heartbeat_interval").as_int64());
			/* Set the heartbeating to begin */
			m_heartbeat_timer.expires_after(chrono::milliseconds(cUtils::Random(0, m_heartbeat_interval.count())));
			m_heartbeat_timer.async_wait([this](const beast::error_code& ec) { on_expire(ec); });
			/* If there is an active session, try to resume, otherwise identify */
			m_session_id.empty() ? identify() : resume();
			break;
		case OP_HEARTBEAT_ACK:
			/* Acknowledge heartbeat */
			m_heartbeat_ack = true;
			break;
	}
} catch (const std::exception& e) {
	cUtils::PrintErr("An error occurred while reading from the gateway: {}", e.what());
	close();
} catch (...) {
	cUtils::PrintErr("An error occurred while reading from the gateway.");
	close();
}
/* ================================================================================================================== */
void
cGateway::implementation::on_write(const beast::error_code& ec) {
	m_async_status &= ~ASYNC_WRITE;
	/* If the operation was canceled, simply return */
	if (m_async_status & ASYNC_CLOSE || ec == asio::error::operation_aborted)
		return;
	/* If an error occurs, close the stream */
	if (ec) {
		const char* err = ec.message(m_err_msg, std::size(m_err_msg));
		cUtils::PrintErr("An error occurred while writing to the gateway{}{}", *err ? ": " : ".", err);
		return close();
	}
	/* If all is good, pop the message that was just sent... */
	m_queue.pop_front();
	/* ...and if the queue isn't empty, send the next message */
	if (!m_queue.empty()) {
		m_ws->async_write(asio::buffer(m_queue.front()), [this](const beast::error_code& ec, std::size_t) { on_write(ec); });
		m_async_status |= ASYNC_WRITE;
	}
}
/* ================================================================================================================== */
void
cGateway::implementation::on_expire(const beast::error_code& ec) {
	/* If the operation was canceled, simply return */
	if (m_async_status & ASYNC_CLOSE || ec) return;
	/* If the last heartbeat wasn't acknowledged, close the stream */
	if (!m_heartbeat_ack) return close();
	/* Send a heartbeat */
	heartbeat();
	/* Reset the timer */
	m_heartbeat_timer.expires_after(m_heartbeat_interval);
	m_heartbeat_timer.async_wait([this](const beast::error_code& ec) { on_expire(ec); });
}
/* ================================================================================================================== */
void
cGateway::implementation::run_session() try {
	using namespace std::chrono_literals;
	/* Switch to the WebSocket strand to safely access the cached urls */
	co_await ResumeOnWebSocketStrand();
	std::string url;
	if (!m_resume_gateway_url.empty()) {
		url = m_resume_gateway_url;
	} else if (!m_cached_gateway_url.empty()) {
		url = m_cached_gateway_url;
	} else {
		const json::value v = co_await DiscordGet("/gateway/bot");
		url = json::value_to<std::string>(v.at("url"));
		co_await ResumeOnWebSocketStrand();
		m_cached_gateway_url = url;
	}
	/* Retrieve the host from the url */
	std::string_view host = url;
	if (auto f = host.find("://"); f != std::string_view::npos)
		host.remove_prefix(f + 3);
	/* Helper macros to wrap handler bodies in a try-catch */
#define HANDLER_BEGIN(ec) { if (ec) return retry(ec.message(m_err_msg, std::size(m_err_msg))); try
#define HANDLER_END catch (const std::exception& ex) { retry(ex.what()); } catch (...) { retry(); }}
	/* Switch to the HTTP strand and resolve host */
	co_await ResumeOnEventStrand();
	m_resolver.async_resolve(host, "https", asio::bind_executor(m_ws_strand, [this, host = (std::string)host](const beast::error_code& ec, asio::ip::tcp::resolver::results_type results) mutable HANDLER_BEGIN(ec) {
		/* Create a WebSocket stream and connect to the resolved host */
		m_ws = std::make_unique<beast::websocket::stream<beast::ssl_stream<beast::tcp_stream>>>(m_ws_strand, m_ctx);
		beast::get_lowest_layer(*m_ws).expires_after(30s);
		beast::get_lowest_layer(*m_ws).async_connect(results, [this, host = std::move(host)](const beast::error_code& ec, const asio::ip::tcp::endpoint& ep) HANDLER_BEGIN(ec) {
			/* Perform the SSL handshake */
			m_ws->next_layer().async_handshake(asio::ssl::stream_base::client, [this, host = fmt::format("{}:{}", host, ep.port())](const beast::error_code& ec) HANDLER_BEGIN(ec) {
				/* Use the recommended timout period for the websocket stream */
				beast::get_lowest_layer(*m_ws).expires_never();
				m_ws->set_option(beast::websocket::stream_base::timeout::suggested(beast::role_type::client));
				/* Set HTTP header fields for the handshake */
				m_ws->set_option(beast::websocket::stream_base::decorator([&host](beast::websocket::request_type& r) {
					r.set(beast::http::field::host, host);
					r.set(beast::http::field::user_agent, "GreekBot");
				}));
				/* Perform the WebSocket handshake */
				m_ws->async_handshake(host, "/?v=" DISCORD_API_VERSION_STR "&encoding=json&compress=zlib-stream", [this](const beast::error_code& ec) HANDLER_BEGIN(ec) {
					/* Start the asynchronous read operation */
					m_ws->async_read(m_buffer, [this](const beast::error_code& ec, std::size_t size) { on_read(ec, size); });
					m_async_status = ASYNC_READ;
				} HANDLER_END);
			} HANDLER_END);
		} HANDLER_END);
	} HANDLER_END));
} catch (...) {
	asio::defer(m_ws_strand, [this, ex = std::current_exception()] {
		try {
			std::rethrow_exception(ex);
		} catch (const std::exception& e) {
			retry(e.what());
		} catch (...) {
			retry();
		}
	});
}
/* ================================================================================================================== */
void
cGateway::implementation::retry(const char* err) {
	using namespace std::chrono_literals;
	/* Reset the stream and the cached gateway url */
	m_ws.reset();
	m_cached_gateway_url.clear();
	/* Copy the message string */
	if (!err) {
		std::strcpy(m_err_msg, "Connection error.");
	} else if (err != m_err_msg) {
		std::strncpy(m_err_msg, err, std::size(m_err_msg));
		m_err_msg[std::size(m_err_msg) - 1] = 0;
	}
	struct {
		implementation& self; int count;
		void operator()() {
			if (count > 0) {
				cUtils::PrintErr<'\r'>("{} Retrying in {}s ", self.m_err_msg, count);
				self.m_heartbeat_timer.expires_after(1s);
				self.m_heartbeat_timer.async_wait(*this);
			} else {
				cUtils::PrintErr<'\n'>("{} Retrying...     ", self.m_err_msg);
				self.run_session();
			}
		}
		void operator()(const beast::error_code&) {
			count--;
			(*this)();
		}
	} _{ *this, 10 }; _();
}
/* ================================================================================================================== */
void
cGateway::implementation::run_context() {
	/* In an exception somehow escapes the async loop, report and retry */
	for (;;) {
		try {
			m_ioc.run();
			break;
		} catch (const std::exception& e) {
			cUtils::PrintErr("An unhandled exception escaped main loop: {}", e.what());
		} catch (...) {
			cUtils::PrintErr("An unhandled exception escaped main loop.");
		}
		asio::dispatch(m_ws_strand, [this] {
			m_ws ? close() : run_session();
		});
	}
	cUtils::PrintLog("exiting run_context()...");
}
/* ================================================================================================================== */
void
cGateway::implementation::Run() {
	run_session();
	std::thread t([this] { run_context(); });
	run_context();
	t.join();
}