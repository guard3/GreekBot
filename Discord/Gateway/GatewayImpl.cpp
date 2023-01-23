#include "GatewayImpl.h"
#include "GatewayInfo.h"
#include "Event.h"
#include <future>

/* ================================================================================================================== */
cGateway::implementation::implementation(cGateway* p, const char* t, eIntent i) :
		m_parent(p),
		m_ctx(asio::ssl::context::tlsv13_client),
		m_ws_resolver(m_ws_ioc),
		m_http_resolver(m_http_ioc),
		m_work(asio::make_work_guard(m_http_ioc)),
		m_work_thread([this]() { m_http_ioc.run(); }),
		m_http_auth(cUtils::Format("Bot %s", t)),
		m_intents(i),
		m_last_sequence(0),
		m_heartbeat_exit(false),
		m_heartbeat_ack(false),
		m_parser(&m_mr) {
	/* Set SSL context to verify peers */
	m_ctx.set_default_verify_paths();
	m_ctx.set_verify_mode(asio::ssl::verify_peer);
	/* Initialize zlib inflate stream */
	memset(&m_inflate_stream, 0, sizeof(m_inflate_stream));
	if (Z_OK != inflateInit(&m_inflate_stream))
		throw std::runtime_error("Could not initialize inflate stream");
}
cGateway::implementation::~implementation() {
	/* Deinitialize the zlib inflate stream */
	inflateEnd(&m_inflate_stream);
	/* Allow m_http_ioc.run() to exit */
	m_work.reset();
	/* Join the worker thread */
	m_work_thread.join();
}
/* ================================================================================================================== */
void
cGateway::implementation::send(std::string msg) {
	/* Make sure we're running on the io_context's implicit strand */
	if (!m_ws_ioc.get_executor().running_in_this_thread())
		return asio::post(m_ws_ioc, [this, s = std::move(msg)] { send(s); });
	/* Push the new message at the end of the message queue */
	m_queue.push_back(std::move(msg));
	/* If there's no other message in the queue, start the asynchronous send operation */
	if (m_queue.size() == 1)
		m_ws->async_write(asio::buffer(m_queue.front()), [this](beast::error_code ec, size_t size) { on_write(ec, size); });
}
/* ================================================================================================================== */
void
cGateway::implementation::on_read(beast::error_code ec, size_t bytes_read) {
	/* If an error occurred, return and let the websocket stream close */
	if (ec)
		return;
	try {
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
						default:
							throw std::runtime_error(m_inflate_stream.msg);
						case Z_OK:
						case Z_STREAM_END:
							m_parser.write((char*)m_inflate_buffer, INFLATE_BUFFER_SIZE - m_inflate_stream.avail_out);
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
		json::value v = m_parser.release();
		/* Start the next asynchronous read operation to keep listening for more events */
		m_ws->async_read(m_buffer, [this](beast::error_code ec, size_t size) { on_read(ec, size); });
#ifdef GW_LOG_LVL_2
		cUtils::PrintLog(json::serialize(v));
#endif
		/* Process the event */
		switch (v.at("op").as_int64()) {
			case OP_DISPATCH:
				/* Process event */
				on_event(std::move(v));
				break;
			case OP_HEARTBEAT:
				/* Send a heartbeat immediately */
				heartbeat();
				break;
			case OP_RECONNECT:
				/* Let the server close the connection gracefully */
				break;
			case OP_INVALID_SESSION:
				/* Wait a random amount of time */
				std::this_thread::sleep_for(std::chrono::milliseconds(cUtils::Random(1000, 5000)));
				/* Try to resume the session */
				if (v.at("d").as_bool()) {
					resume();
					break;
				}
				/* Otherwise, reset session and identify */
				m_last_sequence.store(0);
				m_session_id.clear();
				identify();
				break;
			case OP_HELLO:
				/* Immediately start heartbeating */
				start_heartbeating(chrono::milliseconds(v.at("d").at("heartbeat_interval").as_int64()));
				/* If there is an active session, try to resume, otherwise identify */
				m_session_id.empty() ? identify() : resume();
				break;
			case OP_HEARTBEAT_ACK:
				/* Acknowledge heartbeat */
				m_heartbeat_ack.store(true);
				break;
		}
	}
	catch (const std::exception& e) {
		cUtils::PrintErr("Error parsing received gateway payload: %s", e.what());
	}
	catch (...) {
		cUtils::PrintErr("Error parsing received gateway payload: %s", "An exception was thrown");
	}
	/* Reset json parser for next call */
	m_parser.reset(&m_mr);
}
/* ================================================================================================================== */
void
cGateway::implementation::on_write(beast::error_code ec, size_t size) {
	/* If an error occurred, return and let the websocket stream close */
	if (ec) return;
	/* Pop the message that was just sent */
	m_queue.pop_front();
	/* If the queue isn't clear, continue the asynchronous send operation */
	if (!m_queue.empty())
		m_ws->async_write(asio::buffer(m_queue.front()), [this](beast::error_code ec, size_t size) { on_write(ec, size); });
}
/* ================================================================================================================== */
void
cGateway::implementation::run_session(const std::string& url) {
	/* The websocket close code and reason */
	int         close_code = -1;
	std::string close_msg;
	try {
		/* Resolve host from url */
		auto f = url.find("://");
		std::string host = f == std::string::npos ? url : url.substr(f + 3);
		/* Create a websocket stream */
		m_ws = cHandle::MakeUnique<beast::websocket::stream<beast::ssl_stream<beast::tcp_stream>>>(m_ws_ioc, m_ctx);
		/* Set a timeout and make a connection to the resolved IP address */
		beast::get_lowest_layer(*m_ws).expires_after(std::chrono::seconds(30));
		auto ep = beast::get_lowest_layer(*m_ws).connect(m_ws_resolver.resolve(host, "https"));
		/* Append resolved port number to host name */
		host = cUtils::Format("%s:%d", host, ep.port());
		/* Set a timeout and perform an SSL handshake */
		beast::get_lowest_layer(*m_ws).expires_after(std::chrono::seconds(30));
		m_ws->next_layer().handshake(asio::ssl::stream_base::client);
		/* Use the recommended timout period for the websocket stream */
		beast::get_lowest_layer(*m_ws).expires_never();
		m_ws->set_option(beast::websocket::stream_base::timeout::suggested(beast::role_type::client));
		/* Set HTTP header fields for the handshake */
		m_ws->set_option(beast::websocket::stream_base::decorator([&](beast::websocket::request_type& r) {
			r.set(beast::http::field::host, host);
			r.set(beast::http::field::user_agent, "GreekBot");
		}));
		/* Perform websocket handshake */
		m_ws->handshake(host, cUtils::Format("/?v=%d&encoding=json&compress=zlib-stream", DISCORD_API_VERSION));
		/* Start the asynchronous read operation */
		m_ws->async_read(m_buffer, [this](beast::error_code ec, size_t size) { on_read(ec, size); });
		m_ws_ioc.run();
		/* When the websocket stream closes, save the reason */
		close_code = m_ws->reason().code;
		close_msg  = m_ws->reason().reason.c_str();
	}
	catch (const std::exception& e) {
		close_msg = e.what();
	}
	catch (...) {
		close_msg = "An error occurred";
	}
	/* Reset the websocket context for a subsequent run() call */
	m_ws_ioc.restart();
	/* Stop heartbeating */
	stop_heartbeating();
	/* If the websocket close reason doesn't permit reconnecting, throw */
	switch (const char* reason; close_code) {
		case -1:
			reason = "Error establishing connection: ";
			goto LABEL_THROW;
		case 4004:
		case 4010:
		case 4011:
		case 4012:
		case 4013:
		case 4014:
			reason = "Closing connection: ";
		LABEL_THROW:
			throw xGatewayError(reason + close_msg, close_code);
	}
	/* Reset the zlib inflate stream */
	if (Z_OK != inflateReset(&m_inflate_stream))
		throw std::runtime_error(m_inflate_stream.msg);
}
/* ================================================================================================================== */
cGatewayInfo
cGateway::implementation::get_gateway_info() {
	return cGatewayInfo(m_parent->DiscordGet("/gateway/bot").Wait());
}
/* ================================================================================================================== */
cTask<>
cGateway::implementation::ResumeOnEventThread() {
	struct awaitable {
		asio::io_context& ioc;
		bool await_ready() { return ioc.get_executor().running_in_this_thread(); }
		void await_suspend(std::coroutine_handle<> h) { asio::post(ioc, [h]() { h(); }); }
		void await_resume() {}
	};
	co_await awaitable{m_http_ioc};
}
/* ================================================================================================================== */
void
cGateway::implementation::Run() {
	/* Show a message at exit */
	class _ { public: ~_() { cUtils::PrintLog("Exiting..."); } } show_message_at_exit;
	/* Start the gateway loop */
	for (int timeout = 0;;) {
		try {
			/* Retrieve gateway info */
			cGatewayInfo g = get_gateway_info();
			/* Connect to the gateway and run for a session */
			run_session(g.GetUrl());
			/* Reset error timeout */
			timeout = 0;
		}
		catch (const xGatewayError& e) {
			cUtils::PrintErr(e.what());
			return;
		}
		catch (const xDiscordError& e) {
			cUtils::PrintErr("Couldn't retrieve gateway info");
			cUtils::PrintErr(e.what());
			break;
		}
		catch (const std::exception& e) {
			switch (timeout += 5) {
				case 20:
					cUtils::PrintErr("");
					cUtils::PrintErr(e.what());
					return;
				case 5:
					cUtils::PrintErr("Couldn't retrieve gateway info");
				default:
					for (int tick = timeout; tick >= 0; --tick) {
						cUtils::PrintErr<'\r'>("Retrying in %ds ", tick);
						std::this_thread::sleep_for(std::chrono::seconds(1));
					}
			}
		}
	}
}