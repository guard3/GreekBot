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
	m_heartbeat_ack(false) {
	/* Set SSL context to verify peers */
	m_ctx.set_default_verify_paths();
	m_ctx.set_verify_mode(asio::ssl::verify_peer);
	/* Initialize zlib inflate stream */
	memset(&m_inflate_stream, 0, sizeof(m_inflate_stream));
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
	/* Deinitialize the zlib inflate stream */
	inflateEnd(&m_inflate_stream);
	// TODO: make sure that m_ioc is done
	// ...
}
/* ================================================================================================================== */
void
cGateway::implementation::send(std::string msg) {
	/* Push the new message at the end of the message queue */
	m_queue.push_back(std::move(msg));
	/* If there's no other message in the queue, start the asynchronous send operation */
	if (m_queue.size() == 1)
		m_ws->async_write(asio::buffer(m_queue.front()), [this](const beast::error_code& ec, size_t) { on_write(ec); });
}
/* ================================================================================================================== */
void
cGateway::implementation::close() {
	/* Stop heartbeating to prevent writing further data to the stream */
	m_heartbeat_timer.cancel();
	/* Close the stream */
	auto ws = m_ws.get();
	ws->async_close(beast::websocket::close_code::none, [this, ws = std::move(m_ws)](const beast::error_code& ec) {
		/* Check websocket close code */
		switch (ws->reason().code) {
			case 4004: // Authentication failed
			case 4010: // Invalid shard
			case 4011: // Sharding required
			case 4012: // Invalid API version
			case 4013: // Invalid intent(s)
			case 4014: // Disallowed intent(s)
				/* Display error message */
				cUtils::PrintErr("Fatal gateway error: {}", ws->reason().reason.c_str());
				/* Cancel the http timer to stop giving work to the io context */
				asio::defer(m_http_strand, [this] { m_http_timer.cancel(); });
				return;
			case 4007: // Invalid seq
				/* Reset session */
				m_last_sequence = 0;
				m_session_id.clear();
				m_resume_gateway_url.clear();
			default:
				break;
		}
		/* Create a new session to keep the program loop going */
		run_session();
	});
	/* Reset resources used by the WebSocket session */
	m_queue.clear();
	m_buffer.clear();
	inflateReset(&m_inflate_stream);
}

void
cGateway::implementation::on_read(const beast::error_code& ec, size_t bytes_read) try {
	/* If reading was cancelled, simply return */
	if (ec == asio::error::operation_aborted || !m_ws) return;
	/* If the server closed the connection gracefully, close the stream */
	if (ec == beast::websocket::error::closed) return close();
	/* If any other error occurs, throw */
	if (ec) throw std::system_error(ec);
	/* Reset the parser prior to parsing a new JSON */
	m_parser.reset();
	/* Check for Z_SYNC_FLUSH suffix and decompress if necessary */
	char* in = (char*)m_buffer.data().data();
	if (bytes_read >= 4 ) {
		if (0 == memcmp(in + bytes_read - 4, "\x00\x00\xFF\xFF", 4)) {
			m_inflate_stream.avail_in = bytes_read;
			m_inflate_stream.next_in  = (Byte*)in;
			do {
				m_inflate_stream.avail_out = INFLATE_BUFFER_SIZE;
				m_inflate_stream.next_out  = m_inflate_buffer;
				switch (inflate(&m_inflate_stream, Z_NO_FLUSH)) {
					case Z_OK:
					case Z_STREAM_END:
						m_parser.write((char*)m_inflate_buffer, INFLATE_BUFFER_SIZE - m_inflate_stream.avail_out);
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
	m_ws->async_read(m_buffer, [this](beast::error_code ec, size_t size) { on_read(ec, size); });
#ifdef GW_LOG_LVL_2
	cUtils::PrintLog("{}", json::serialize(v));
#endif
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
				m_heartbeat_timer.async_wait([this](beast::error_code ec){ on_expire(ec); });
			}
			/* If cancel() returns 0, then a heartbeat is already queued to be sent, so we don't need to do anything */
			break;
		case OP_INVALID_SESSION:
			/* If the current session can't be resumed, reset it */
			if (!v.at("d").as_bool()) {
				m_last_sequence = 0;
				m_session_id.clear();
				m_resume_gateway_url.clear();
			}
		case OP_RECONNECT:
			close();
			break;
		case OP_HELLO:
			/* Update heartbeat interval */
			m_heartbeat_ack = true;
			m_heartbeat_interval = chrono::milliseconds(v.at("d").at("heartbeat_interval").as_int64());
			/* Set the heartbeating to begin */
			m_heartbeat_timer.expires_after(chrono::milliseconds(cUtils::Random(0, m_heartbeat_interval.count())));
			m_heartbeat_timer.async_wait([this](beast::error_code ec) { on_expire(ec); });
			/* If there is an active session, try to resume, otherwise identify */
			m_session_id.empty() ? identify() : resume();
			break;
		case OP_HEARTBEAT_ACK:
			/* Acknowledge heartbeat */
			m_heartbeat_ack = true;
			break;
	}
} catch (...) {
	/* In the case of an error, close the stream */
	close();
}
/* ================================================================================================================== */
void
cGateway::implementation::on_write(const beast::error_code& ec) {
	/* If the operation was canceled, simply return */
	if (ec == asio::error::operation_aborted || !m_ws) return;
	/* If an error occurred, close the stream */
	if (ec) return close();
	/* Pop the message that was just sent */
	m_queue.pop_front();
	/* If the queue isn't empty, send the next message */
	if (!m_queue.empty())
		m_ws->async_write(asio::buffer(m_queue.front()), [this](const beast::error_code& ec, size_t) { on_write(ec); });
}
/* ================================================================================================================== */
void
cGateway::implementation::on_expire(const beast::error_code& ec) {
	/* If the operation was canceled, simply return */
	if (ec == asio::error::operation_aborted || !m_ws) return;
	/* If an error occurred or the last heartbeat wasn't acknowledged, close the stream */
	if (ec || !m_heartbeat_ack) return close();
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
	/* Switch to the WebSocket strand to safely access the cached resume url */
	co_await ResumeOnWebSocketStrand();
	std::string url;
	if (m_resume_gateway_url.empty()) {
		const json::value v = co_await DiscordGet("/gateway/bot");
		url = json::value_to<std::string>(v.at("url"));
	} else {
		url = m_resume_gateway_url;
		co_await ResumeOnEventThread();
	}
	/* At this point we are on the HTTP strand, so we can safely use the resolver */
	std::string_view host = url;
	if (auto f = host.find("://"); f != std::string_view::npos)
		host.remove_prefix(f + 3);
	/* Resolve host */
	m_resolver.async_resolve(host, "https", asio::bind_executor(m_ws_strand, [this, host = std::string{ host.data(), host.size() }](const beast::error_code& ec, const asio::ip::tcp::resolver::results_type& results) mutable {
		try {
			/* On error, throw */
			if (ec) throw std::system_error(ec);
			/* Create a WebSocket stream and connect to the resolved host */
			m_ws = std::make_unique<beast::websocket::stream<beast::ssl_stream<beast::tcp_stream>>>(m_ws_strand, m_ctx);
			beast::get_lowest_layer(*m_ws).expires_after(30s);
			beast::get_lowest_layer(*m_ws).async_connect(results, [this, host = std::move(host)](const beast::error_code& ec, const asio::ip::tcp::endpoint& ep) {
				try {
					/* On error, throw */
					if (ec) throw std::system_error(ec);
					/* Perform the SSL handshake */
					m_ws->next_layer().async_handshake(asio::ssl::stream_base::client, [this, host = fmt::format("{}:{}", host, ep.port())](const beast::error_code& ec) {
						try {
							/* On error, throw */
							if (ec) throw std::system_error(ec);
							/* Use the recommended timout period for the websocket stream */
							beast::get_lowest_layer(*m_ws).expires_never();
							m_ws->set_option(beast::websocket::stream_base::timeout::suggested(beast::role_type::client));
							/* Set HTTP header fields for the handshake */
							m_ws->set_option(beast::websocket::stream_base::decorator([&](beast::websocket::request_type& r) {
								r.set(beast::http::field::host, host);
								r.set(beast::http::field::user_agent, "GreekBot");
							}));
							/* Perform the WebSocket handshake */
							m_ws->async_handshake(host, fmt::format("/?v={}&encoding=json&compress=zlib-stream", DISCORD_API_VERSION), [this](const beast::error_code& ec) {
								try {
									/* On error, throw */
									if (ec) throw std::system_error(ec);
									/* Start the asynchronous read operation */
									m_ws->async_read(m_buffer, [this](beast::error_code ec, size_t size) { on_read(ec, size); });
								} catch (...) {
									m_ws = nullptr;
									retry(5, "Connection error.");
								}
							});
						} catch (...) {
							m_ws = nullptr;
							retry(5, "Connection error.");
						}
					});
				} catch (...) {
					m_ws = nullptr;
					retry(5, "Connection error.");
				}
			});
		} catch (...) {
			m_ws = nullptr;
			retry(5, "Connection error.");
		}
	}));
} catch (...) {
	asio::dispatch(m_ws_strand, [this] {
		m_ws = nullptr;
		retry(5, "Connection error.");
	});
}

void
cGateway::implementation::retry(int count, std::string msg) {
	using namespace std::chrono_literals;
	if (count == 0) {
		cUtils::PrintErr<'\n'>("{} Retrying...    ", msg);
		run_session();
	} else {
		cUtils::PrintErr<'\r'>("{} Retrying in {}s", msg, count);
		m_heartbeat_timer.expires_after(1s);
		m_heartbeat_timer.async_wait([this, c = count - 1, msg = std::move(msg)](const beast::error_code& ec) {
			if (ec) throw std::system_error(ec);
			retry(c, std::move(msg));
		});
	}
}

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