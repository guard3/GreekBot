#include "GatewayImpl.h"
#include "Utils.h"

#include <print>
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
	m_ctx(net::ssl::context::tlsv13_client),
	m_ws_strand(net::make_strand(m_ioc)),
	m_http_strand(net::make_strand(m_ioc)),
	m_ws_resolver(m_ws_strand),
	m_http_resolver(m_http_strand),
	m_http_auth(std::format("Bot {}", t)),
	m_http_timer(m_http_strand),
	m_intents(i) {
	/* Set SSL context to verify peers */
	m_ctx.set_default_verify_paths();
	m_ctx.set_verify_mode(net::ssl::verify_peer);
}
/* ================================================================================================================== */
void
cGateway::implementation::send(std::shared_ptr<websocket_session> sess, std::string msg) {
	/* Push the new message at the end of the message queue */
	auto& m = sess->queue.emplace_back(std::move(msg));
	/* If there's no other message in the queue, start the asynchronous send operation */
	if (auto p = sess.get(); p->queue.size() == 1)
		p->stream.async_write(net::buffer(m), [this, sess = std::move(sess)](const sys::error_code &ec, std::size_t) mutable { on_write(std::move(sess), ec); });
}
/* ================================================================================================================== */
void
cGateway::implementation::restart(std::shared_ptr<websocket_session> sess) noexcept {
	if (sess->closing) return;

	auto pSess = sess.get();
	pSess->closing = true;    // Mark the current session as closing to cancel any pending handlers associated with it
	pSess->hb_timer.cancel(); // Stop heartbeating to prevent writing further data to the stream
	pSess->stream.async_close(beast::websocket::close_code::none, [_ = std::move(sess)](const sys::error_code&) {});

	m_ws_session.reset();
	if (!m_ws_terminating)
		run_session();
}
void
cGateway::implementation::on_close(std::shared_ptr<websocket_session> sess) noexcept {
	if (sess->closing) return;

	sess->closing = true;    // Mark the current session as closing to cancel any pending handlers associated with it
	sess->hb_timer.cancel(); // Stop heartbeating to prevent writing further data to the stream

	m_ws_session.reset();
	if (!m_ws_terminating)
		run_session();
}
/* ================================================================================================================== */
void
cGateway::implementation::on_read(std::shared_ptr<websocket_session> sess, const sys::error_code& ec, std::size_t bytes_read) noexcept try {
	/* If the session was closed gracefully by the server... */
	if (ec == beast::websocket::error::closed) {
		switch (auto& rsn = sess->stream.reason(); rsn.code) {
			case 4004: // Authentication failed
			case 4010: // Invalid shard
			case 4011: // Sharding required
			case 4012: // Invalid API version
			case 4013: // Invalid intent(s)
			case 4014: // Disallowed intent(s)
				/* Report the close reason as the error message */
				cUtils::PrintErr("Fatal gateway error{}{}", rsn.reason.empty() ? "." : ": ", rsn.reason.c_str());
				/* Continue closing without creating a new session */
				net::defer(m_http_strand, [this]() {
					m_http_terminate = true;
					m_http_timer.cancel();
				});
				m_ws_terminating = true;
				return on_close(std::move(sess));
			case 4007: // Invalid seq
				/* Reset session */
				m_last_sequence = 0;
				m_session_id.clear();
				m_resume_gateway_url.clear();
			default:
				/* Continue closing */
				return on_close(std::move(sess));
		}
	}
	/* If the connection was unexpectedly closed... */
	if (ec == net::error::eof || ec == net::ssl::error::stream_truncated)
		return on_close(std::move(sess));
	/* If the operation was canceled, simply return */
	if (ec == sys::errc::operation_canceled || sess->closing)
		return;
	/* If any other error occurs, report it and close the connection */
	if (ec)
		throw sys::system_error(ec);
	/* Reset the JSON parser and feed it the received message */
	m_parser.reset();
	char* in = (char*)sess->buffer.data().data();
	if (bytes_read >= 4 && 0 == std::memcmp(in + bytes_read - 4, "\x00\x00\xFF\xFF", 4)) {
		/* If the message ends in the Z_SYNC_FLUSH suffix, decompress */
		auto& inflate_stream = sess->inflate_stream;
		auto& inflate_buffer = sess->inflate_buffer;

		inflate_stream.avail_in = bytes_read;
		inflate_stream.next_in  = (Byte*)in;
		do {
			inflate_stream.avail_out = std::size(inflate_buffer);
			inflate_stream.next_out  = inflate_buffer;
			switch (inflate(&inflate_stream, Z_NO_FLUSH)) {
				case Z_OK:
				case Z_STREAM_END:
					m_parser.write((char*)inflate_buffer, std::size(inflate_buffer) - inflate_stream.avail_out);
					break;
				case Z_MEM_ERROR:
					throw std::bad_alloc();
				default:
					throw std::runtime_error(inflate_stream.msg ? inflate_stream.msg : "Zlib error");
			}
		} while (inflate_stream.avail_out == 0);
	} else {
		/* If the message isn't compressed, directly feed it into the json parser */
		m_parser.write(in, bytes_read);
	}
	/* Consume all read bytes from the dynamic buffer */
	sess->buffer.consume(bytes_read);
	/* Release the parsed json value */
	const json::value v = m_parser.release();
	cUtils::PrintDbg("{}", json::serialize(v));
	/* Start the next asynchronous read operation to keep listening for more events */
	sess->stream.async_read(sess->buffer, [this, sess](const sys::error_code& ec, std::size_t size) mutable { on_read(std::move(sess), ec, size); });
	/* Process the event */
	switch (v.at("op").to_number<int>()) {
		case OP_DISPATCH:
			/* Process event */
			process_event(sess, v);
			break;
		case OP_HEARTBEAT:
			/* Cancel any pending heartbeats */
			if (sess->hb_timer.cancel() != 0) {
				/* Send a heartbeat immediately */
				heartbeat(sess);
				/* Resume heartbeating */
				sess->hb_timer.expires_after(sess->hb_interval);
				sess->hb_timer.async_wait([this, sess](const sys::error_code& ec) mutable { on_expire(std::move(sess), ec); });
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
			restart(sess);
		case OP_RECONNECT:
			/* ...and do nothing and wait for the server to close the WebSocket session */
			break;
		case OP_HELLO:
			/* Update heartbeat interval */
			sess->hb_ack = true;
			sess->hb_interval = milliseconds(v.at("d").at("heartbeat_interval").to_number<milliseconds::rep>());
			/* Set the heartbeating to begin */
			sess->hb_timer.expires_after(milliseconds(cUtils::Random(0, sess->hb_interval.count())));
			sess->hb_timer.async_wait([this, sess](const sys::error_code& ec) mutable { on_expire(std::move(sess), ec); });
			/* If there is an active session, try to resume, otherwise identify */
			m_session_id.empty() ? identify(sess) : resume(sess);
			break;
		case OP_HEARTBEAT_ACK:
			/* Acknowledge heartbeat */
			sess->hb_ack = true;
			on_heartbeat();
			break;
	}
} catch (const std::exception& e) {
	restart(std::move(sess));
	cUtils::PrintErr("An error occurred while reading from the gateway{}{}", *e.what() ? ": " : ".", e.what());
} catch (...) {
	restart(std::move(sess));
	cUtils::PrintErr("An error occurred while reading from the gateway{}{}", ".", "");
}
/* ================================================================================================================== */
void
cGateway::implementation::on_write(std::shared_ptr<websocket_session> sess, const sys::error_code& ec) noexcept try {
	/* If the operation was canceled, simply return */
	if (ec == sys::errc::operation_canceled || sess->closing)
		return;
	/* If an error occurs, close the stream */
	if (ec)
		throw sys::system_error(ec);
	/* If all is good, pop the message that was just sent... */
	sess->queue.pop_front();
	/* ...and if the queue isn't empty, send the next message */
	if (!sess->queue.empty())
		sess->stream.async_write(net::buffer(sess->queue.front()), [this, sess](const sys::error_code& ec, std::size_t) mutable { on_write(std::move(sess), ec); });
} catch (const std::exception& ex) {
	restart(std::move(sess));
	cUtils::PrintErr("An error occurred while writing to the gateway{}{}", *ex.what() ? ": " : ".", ex.what());
} catch (...) {
	restart(std::move(sess));
	cUtils::PrintErr("An error occurred while writing to the gateway{}{}", ".", "");
}
/* ================================================================================================================== */
void
cGateway::implementation::on_expire(std::shared_ptr<websocket_session> sess, const sys::error_code& ec) {
	/* If the operation was canceled, simply return */
	if (ec || sess->closing) return;
	/* If the last heartbeat wasn't acknowledged, close the stream */
	if (!sess->hb_ack) return restart(std::move(sess));
	/* Send a heartbeat */
	heartbeat(sess);
	/* Reset the timer */
	auto pSess = sess.get();
	pSess->hb_timer.expires_after(pSess->hb_interval);
	pSess->hb_timer.async_wait([this, sess = std::move(sess)](const sys::error_code& ec) mutable { on_expire(std::move(sess), ec); });
}
/* ================================================================================================================== */
void
cGateway::implementation::run_session() noexcept try {
	co_await ResumeOnWebSocketStrand();
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
	if (pUrl->scheme_id() != urls::scheme::wss)
		throw std::runtime_error(std::format("Not a WebSocket url: {}", std::string_view{ pUrl->data(), pUrl->size() }));

	net::co_spawn(m_ws_strand, [this, url = *pUrl]() mutable -> net::awaitable<void> {
		using namespace std::chrono_literals;
		/* Create a new websocket session */
		auto sess = std::make_shared<websocket_session>(m_ws_strand, m_ctx);
		auto& ws_layer = sess->stream;
		auto& ssl_layer = ws_layer.next_layer();
		auto& tcp_layer = ssl_layer.next_layer();
		/* Resolve host; The WebSocket connection starts with an HTTP request, so we must provide that as a service! */
		auto results = co_await m_ws_resolver.async_resolve(url.encoded_host(), url.has_port() ? url.port() : "https", net::use_awaitable);
		/* Connect to the resolved host and perform the SSL handshake; 30s timeout */
		tcp_layer.expires_after(30s);
		auto endpoint = co_await tcp_layer.async_connect(results, net::use_awaitable);
		co_await ssl_layer.async_handshake(ssl_stream::client, net::use_awaitable);
		/* Use the recommended timout period for the websocket stream */
		tcp_layer.expires_never();
		ws_layer.set_option(beast::websocket::stream_base::timeout::suggested(beast::role_type::client));
		/* Update url with the required parameters */
		if (url.encoded_path().empty())
			url.set_encoded_path("/");
		url.set_port_number(endpoint.port()).encoded_params().append({
			{ "v", DISCORD_API_VERSION_STR }, // Explicitly specify the api version
			{ "encoding", "json"           }, // Encode payloads as json
			{ "compress", "zlib-stream"    }  // Use zlib compression where possible
		});
		/* Set HTTP header fields for the handshake */
		ws_layer.set_option(beast::websocket::stream_base::decorator([&url](beast::websocket::request_type& r) {
			r.set(beast::http::field::host, url.encoded_authority());
			r.set(beast::http::field::user_agent, "GreekBot");
		}));
		/* Perform the WebSocket handshake */
		co_await ws_layer.async_handshake(url.encoded_authority(), url.encoded_resource(), net::use_awaitable);
		/* Start the asynchronous read operation */
		auto pSess = sess.get();
		ws_layer.async_read(pSess->buffer, [this, sess = std::move(sess)](const sys::error_code& ec, std::size_t size) mutable {
			m_ws_session = sess; // Save a weak reference to the session
			on_read(std::move(sess), ec, size);
		});
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
		char temp[256]{};
		try {
			std::rethrow_exception(ex);
		} catch (const std::exception& e) {
			std::strncpy(temp, e.what(), std::size(temp) - 1);
		} catch (...) {}
		/* If the message is empty, use a generic one as a default */
		const char* err = temp[0] ? temp : "Connection error.";
		/* Wait for 10 seconds and restart */
		net::steady_timer timer(m_ws_strand);
		for (int i = 10; i > 0; --i) {
			if (m_ws_terminating)
				co_return;
			cUtils::PrintErr<'\r'>("{} Retrying in {}s ", err, i);
			timer.expires_after(1s);
			co_await timer.async_wait(net::use_awaitable);
		}
		cUtils::PrintErr("{} Retrying...     ", err);
	}, [this](std::exception_ptr) {
		// Any exception that may be thrown is a result of the timer being canceled, which shouldn't ever happen
		// but is ok to ignore in any case, just start a new session
		if (!m_ws_terminating)
			run_session();
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
			cUtils::PrintErr("An unhandled exception escaped main loop{}{}", *e.what() ? ": " : ".", e.what());
		} catch (...) {
			cUtils::PrintErr("An unhandled exception escaped main loop{}{}", ".", "");
		}
		net::dispatch(m_ws_strand, [this] noexcept {
			auto sess = m_ws_session.lock();
			sess ? restart(std::move(sess)) : run_session();
		});
	}
	cUtils::PrintLog("exiting run_context()...");
}
/* ================================================================================================================== */
void
cGateway::implementation::Run() {
	// Catch signals to terminate gracefully
	net::signal_set sigset(m_ws_strand, SIGINT, SIGTSTP, SIGTERM);
	sigset.async_wait([this](const sys::error_code& ec, int sig) {
		if (!ec) {
			char buffer[32];
			cUtils::PrintLog("{} received, terminating...", [sig, &buffer] -> std::string_view {
				switch (sig) {
					case SIGINT: return "SIGINT";
					case SIGTSTP: return "SIGTSTP";
					case SIGTERM: return "SIGTERM";
					default: return { buffer, std::format_to(buffer, "Signal {}", sig) };
				}
			}());
			// Terminate websocket
			m_ws_terminating = true;
			if (auto sess = m_ws_session.lock())
				restart(std::move(sess));
			// Terminate http
			net::post(m_http_strand, [this] {
				m_http_terminate = true;
				m_http_timer.cancel();
			});
		}
	});

	// Start
	run_session();
	std::thread t(&implementation::run_context, this);
	run_context();
	t.join();
}