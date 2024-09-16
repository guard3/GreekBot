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
/* ========== Flags to keep track of the status of async operations of the WebSocket stream ========================= */
enum {
	ASYNC_NONE    = 0,
	ASYNC_READING = 1 << 0,
	ASYNC_WRITING = 1 << 1,
	ASYNC_OPENING = 1 << 2,
	ASYNC_CLOSING = 1 << 3
};
/* ================================================================================================================== */
cGateway::implementation::implementation(cGateway* p, std::string_view t, eIntent i) :
	m_parent(p),
	m_ctx(net::ssl::context::tlsv13_client),
	m_ws_strand(net::make_strand(m_ioc)),
	m_http_strand(net::make_strand(m_ioc)),
	m_resolver(m_http_strand),
	m_async_status(ASYNC_NONE),
	m_http_auth(fmt::format("Bot {}", t)),
	m_http_timer(m_http_strand),
	m_intents(i),
	m_last_sequence(0),
	m_heartbeat_timer(m_ws_strand),
	m_heartbeat_ack(false),
	m_inflate_stream{} {
	/* Set SSL context to verify peers */
	m_ctx.set_default_verify_paths();
	m_ctx.set_verify_mode(net::ssl::verify_peer);
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
	/* This shouldn't ever happen, but ensure that the websocket stream is not null */
	if (m_async_status & (ASYNC_OPENING | ASYNC_CLOSING) || !m_ws)
		return;
	/* Push the new message at the end of the message queue */
	m_queue.push_back(std::move(msg));
	/* If there's no other message in the queue, start the asynchronous send operation */
	if (m_queue.size() == 1) {
		m_ws->async_write(net::buffer(m_queue.front()), [this](const beast::error_code& ec, std::size_t) { on_write(ec); });
		m_async_status |= ASYNC_WRITING;
	}
}
/* ================================================================================================================== */
void
cGateway::implementation::close() noexcept {
	/* If we're already closing, don't do anything */
	if (m_async_status & ASYNC_CLOSING)
		return;
	/* If the websocket stream is null, continue closing right away */
	if (!m_ws)
		return on_close();
	/* Stop heartbeating to prevent writing further data to the stream */
	m_heartbeat_timer.cancel();
	/* Close the stream */
	m_ws->async_close(beast::websocket::close_code::none, [this](auto&&) { on_close(); });
	m_async_status |= ASYNC_CLOSING;
}
void
cGateway::implementation::on_close(bool bRestart) noexcept {
	/* Stop heartbeating to prevent writing further data to the stream */
	if (!(m_async_status & ASYNC_CLOSING)) {
		m_heartbeat_timer.cancel();
		m_async_status |= ASYNC_CLOSING;
	}
	struct {
		implementation& self; bool bRestart;
		void operator()() {
			/* Wait for all async operations to finish */
			if (self.m_async_status & (ASYNC_READING | ASYNC_WRITING))
				return net::defer(self.m_ws_strand, *this);
			/* If we have to exit, simply return and wait for the io context to run out of work */
			if (!bRestart) return;
			/* Reset all resources */
			inflateReset(&self.m_inflate_stream);
			self.m_queue.clear();
			self.m_buffer.clear();
			self.m_ws.reset();
			/* Start a new session */
			self.m_async_status = ASYNC_NONE;
			self.run_session();
		}
	} _{ *this, bRestart }; _();
}
/* ================================================================================================================== */
void
cGateway::implementation::on_read(const beast::error_code& ec, std::size_t bytes_read) try {
	m_async_status &= ~ASYNC_READING;
	/* If the operation was cancelled, simply return */
	if (m_async_status & (ASYNC_OPENING | ASYNC_CLOSING) || ec == net::error::operation_aborted)
		return;
	/* If the connection was unexpectedly closed... */
	if (ec == net::error::eof)
		return on_close();
	/* If the session was closed gracefully by the server... */
	if (ec == beast::websocket::error::closed) {
		if (!m_ws)
			return;
		switch (auto& rsn = m_ws->reason(); rsn.code) {
			case 4004: // Authentication failed
			case 4010: // Invalid shard
			case 4011: // Sharding required
			case 4012: // Invalid API version
			case 4013: // Invalid intent(s)
			case 4014: // Disallowed intent(s)
				/* Cancel the http timer to stop giving work to the io context */
				net::post(m_http_strand, [this] { m_http_timer.cancel(); });
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
	/* Reset the JSON parser and feed it the received message */
	m_parser.reset();
	char* in = (char*)m_buffer.data().data();
	if (bytes_read >= 4 && 0 == std::memcmp(in + bytes_read - 4, "\x00\x00\xFF\xFF", 4)) {
		/* If the message ends in the Z_SYNC_FLUSH suffix, decompress */
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
					throw std::runtime_error(m_inflate_stream.msg ? m_inflate_stream.msg : "Zlib error");
			}
		} while (m_inflate_stream.avail_out == 0);
	} else {
		/* If the message isn't compressed, directly feed it into the json parser */
		m_parser.write(in, bytes_read);
	}
	/* Consume all read bytes from the dynamic buffer */
	m_buffer.consume(bytes_read);
	/* Release the parsed json value */
	const json::value v = m_parser.release();
	cUtils::PrintDbg("{}", json::serialize(v));
	/* Start the next asynchronous read operation to keep listening for more events */
	if (m_ws) {
		m_ws->async_read(m_buffer, [this](const beast::error_code &ec, std::size_t size) { on_read(ec, size); });
		m_async_status |= ASYNC_READING;
	}
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
			m_heartbeat_interval = milliseconds(v.at("d").at("heartbeat_interval").to_number<milliseconds::rep>());
			/* Set the heartbeating to begin */
			m_heartbeat_timer.expires_after(milliseconds(cUtils::Random(0, m_heartbeat_interval.count())));
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
	m_async_status &= ~ASYNC_WRITING;
	/* If the operation was canceled, simply return */
	if (m_async_status & (ASYNC_OPENING | ASYNC_CLOSING) || ec == net::error::operation_aborted)
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
	if (!m_queue.empty() && m_ws) {
		m_ws->async_write(net::buffer(m_queue.front()), [this](const beast::error_code& ec, std::size_t) { on_write(ec); });
		m_async_status |= ASYNC_WRITING;
	}
}
/* ================================================================================================================== */
void
cGateway::implementation::on_expire(const beast::error_code& ec) {
	/* If the operation was canceled, simply return */
	if (m_async_status & (ASYNC_OPENING | ASYNC_CLOSING) || ec) return;
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
cGateway::implementation::run_session() noexcept try {
	co_await ResumeOnWebSocketStrand();
	/* If the websocket stream is already opening, skip */
	if (m_async_status & ASYNC_OPENING)
		co_return;
	/* If, for whatever reason, there are pending async operations, restart */
	if (m_async_status != ASYNC_NONE)
		co_return close();
	m_async_status = ASYNC_OPENING;
	/* Find the appropriate url to connect to */
	urls::url* pUrl;
	if (!m_resume_gateway_url.empty()) {
		pUrl = &m_resume_gateway_url;
	} else if (!m_cached_gateway_url.empty()) {
		pUrl = &m_cached_gateway_url;
	} else {
		const json::value v = co_await DiscordGet("/gateway/bot");
		co_await ResumeOnWebSocketStrand();
		m_cached_gateway_url = urls::parse_uri(v.at("url").as_string()).value();
		pUrl = &m_cached_gateway_url;
	}
	/* Make sure that the url scheme is WSS */
	if (pUrl->scheme_id() != urls::scheme::wss) {
		class _ : public std::exception {
			char m_msg[256];
		public:
			_(const urls::url& url) : m_msg{} {
				fmt::format_to_n(m_msg, std::size(m_msg) - 1, "Invalid gateway URL {}", std::string_view(url.data(), url.size()));
			}
			const char* what() const noexcept override { return m_msg; }
		}; throw _(*pUrl);
	}
	net::co_spawn(m_http_strand, [this, url = *pUrl]() mutable -> net::awaitable<void> {
		using namespace std::chrono_literals;
		/* Resolve host; The WebSocket connection starts with an HTTP request, so we must provide that as service! */
		auto results = co_await m_resolver.async_resolve(url.encoded_host(), url.has_port() ? url.port() : "https", net::bind_executor(m_ws_strand, net::use_awaitable));
		/* Create a WebSocket stream and connect; 30s timeout */
		m_ws = std::make_unique<websocket_stream>(m_ws_strand, m_ctx);
		/* Connect to the resolved host and perform the SSL handshake; 30s timeout */
		beast::get_lowest_layer(*m_ws).expires_after(30s);
		auto endpoint = co_await beast::get_lowest_layer(*m_ws).async_connect(results, net::use_awaitable);
		co_await m_ws->next_layer().async_handshake(ssl_stream::client, net::use_awaitable);
		/* Use the recommended timout period for the websocket stream */
		beast::get_lowest_layer(*m_ws).expires_never();
		m_ws->set_option(beast::websocket::stream_base::timeout::suggested(beast::role_type::client));
		/* Update url with the required parameters */
		if (url.encoded_path().empty())
			url.set_encoded_path("/");
		url.set_port_number(endpoint.port()).encoded_params().append({
			{ "v", DISCORD_API_VERSION_STR }, // Explicitly specify the api version
			{ "encoding", "json"           }, // Encode payloads as json
			{ "compress", "zlib-stream"    }  // Use zlib compression where possible
		});
		/* Set HTTP header fields for the handshake */
		m_ws->set_option(beast::websocket::stream_base::decorator([&url](beast::websocket::request_type& r) {
			r.set(beast::http::field::host, url.encoded_authority());
			r.set(beast::http::field::user_agent, "GreekBot");
		}));
		/* Perform the WebSocket handshake */
		co_await m_ws->async_handshake(url.encoded_authority(), url.encoded_resource(), net::use_awaitable);
		/* Start the asynchronous read operation */
		m_ws->async_read(m_buffer, [this](const beast::error_code& ec, std::size_t size) { on_read(ec, size); });
		m_async_status = ASYNC_READING;
		/* The stream is successfully set up, clear the error timer */
		m_error_started_at = {};
	}, [this](std::exception_ptr ex) { if (ex) retry(ex); });
} catch (...) {
	retry(std::current_exception());
}
/* ================================================================================================================== */
void
cGateway::implementation::retry(std::exception_ptr ex) noexcept {
	net::co_spawn(m_ws_strand, [this, ex]() -> net::awaitable<void> {
		using namespace std::chrono_literals;
		/* If there have been continuous errors for more than a minute, clear the offending url */
		if (auto now = steady_clock::now(); m_error_started_at == steady_clock::time_point{}) {
			m_error_started_at = now;
		} else if (now - m_error_started_at > 1min) {
			(m_resume_gateway_url.empty() ? m_cached_gateway_url : m_resume_gateway_url).clear();
			m_error_started_at = now;
		}
		/* Retrieve the exception message */
		char err[256]{};
		try {
			std::rethrow_exception(ex);
		} catch (const std::exception& e) {
			std::strncpy(err, e.what(), std::size(err) - 1);
		} catch (...) {}
		/* If the message is empty, use a generic one as a default */
		if (err[0] == 0)
			std::strcpy(err, "Connection error.");
		/* Wait for 10 seconds and restart */
		for (int i = 10; i > 0; --i) {
			cUtils::PrintErr<'\r'>("{} Retrying in {}s ", err, i);
			m_heartbeat_timer.expires_after(1s);
			co_await m_heartbeat_timer.async_wait(net::use_awaitable);
		}
		cUtils::PrintErr("{} Retrying...     ", err);
		close();
	}, [](std::exception_ptr ex) {
		if (ex) try {
			std::rethrow_exception(ex);
		} catch (const beast::system_error& e) {
			if (e.code() != net::error::operation_aborted)
				throw;
		}
	});
}
/* ================================================================================================================== */
void
cGateway::implementation::run_context() noexcept {
	/* If an exception somehow escapes the async loop, report and retry */
	for (;;) {
		try {
			m_ioc.run();
			break;
		} catch (const std::exception& e) {
			cUtils::PrintErr("An unhandled exception escaped main loop: {}", e.what());
		} catch (...) {
			cUtils::PrintErr("An unhandled exception escaped main loop.");
		}
		net::dispatch(m_ws_strand, [this] { close(); });
	}
	cUtils::PrintLog("exiting run_context()...");
}
/* ================================================================================================================== */
void
cGateway::implementation::Run() {
	run_session();
	std::thread t(&implementation::run_context, this);
	run_context();
	t.join();
}