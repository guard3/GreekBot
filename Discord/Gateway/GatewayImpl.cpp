#include "GatewayImpl.h"
#include "GatewayInfo.h"
#include "Utils.h"

/* ================================================================================================================== */
cGateway::implementation::implementation(cGateway* p, std::string_view t, eIntent i) :
	m_parent(p),
	m_ctx(asio::ssl::context::tlsv13_client),
	m_ws_resolver(m_ws_ioc),
	m_http_resolver(m_http_ioc),
	m_work(asio::make_work_guard(m_http_ioc)),
	m_work_thread([this]() { m_http_ioc.run(); }),
	m_http_auth(fmt::format("Bot {}", t)),
	m_http_timer(m_http_ioc),
	m_intents(i),
	m_last_sequence(0),
	m_heartbeat_timer(m_ws_ioc),
	m_heartbeat_ack(false) {
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
cGateway::implementation::on_read(const beast::error_code& ec, size_t bytes_read) {
	/* If an error occurs, throw */
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
			m_heartbeat_timer.cancel();
			/* Send a heartbeat immediately */
			heartbeat();
			/* Resume heartbeating */
			m_heartbeat_timer.expires_after(m_heartbeat_interval);
			m_heartbeat_timer.async_wait([this](beast::error_code ec){ on_expire(ec); });
			break;
		case OP_RECONNECT:
			/* Let the server close the connection gracefully */
			break;
		case OP_INVALID_SESSION:
			/* If session is not resumable, reset the current session */
			if (!v.at("d").as_bool()) {
				m_last_sequence = 0;
				m_session_id.clear();
				m_resume_gateway_url.clear();
			}
			/* Stop heartbeating and close the connection */
			m_heartbeat_timer.cancel();
			m_ws->async_close(beast::websocket::close_code::none, [](const beast::error_code& ec) {});
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
}
/* ================================================================================================================== */
void
cGateway::implementation::on_write(const beast::error_code& ec, size_t size) {
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
cGateway::implementation::on_expire(const beast::error_code& ec) {
	/* If the operation was canceled or the previous heartbeat wasn't acknowledged, return and let the io_context run out of work */
	if (ec || !m_heartbeat_ack) return;
	/* Send a heartbeat */
	heartbeat();
	/* Reset the timer */
	m_heartbeat_timer.expires_after(m_heartbeat_interval);
	m_heartbeat_timer.async_wait([this](beast::error_code ec) { on_expire(ec); });
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
		host = fmt::format("{}:{}", host, ep.port());
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
		m_ws->handshake(host, fmt::format("/?v={}&encoding=json&compress=zlib-stream", DISCORD_API_VERSION));
		/* Start the asynchronous read operation */
		m_ws->async_read(m_buffer, [this](beast::error_code ec, size_t size) { on_read(ec, size); });
		m_ws_ioc.run();
		/* When the websocket stream closes, save the reason */
		close_code = m_ws->reason().code;
		close_msg  = m_ws->reason().reason.c_str();
	}
	catch (const std::exception& e) {
		/* Save the error message */
		close_msg = e.what();
		/* Cancel heartbeating */
		m_heartbeat_timer.cancel();
		/* Close the websocket stream */
		m_ws->async_close(beast::websocket::close_code::none, [](const beast::error_code& ec) {});
		/* Exhaust the io_context */
		for (;;) {
			try {
				m_ws_ioc.run();
				break;
			}
			catch (...) {}
		}
	}
	/* Reset the websocket context for a subsequent run() call */
	m_ws_ioc.restart();
	/* Clear any pending messages or unread data */
	m_queue.clear();
	m_buffer.clear();
	/* Check websocket close code */
	switch (close_code) {
		case 4007: // Invalid seq
			m_last_sequence = 0;
			m_session_id.clear();
			m_resume_gateway_url.clear();
			break;
		default:
			break;
		case 4004: // Authentication failed
		case 4010: // Invalid shard
		case 4011: // Sharding required
		case 4012: // Invalid API version
		case 4013: // Invalid intent(s)
		case 4014: // Disallowed intent(s)
			throw xGatewayError(fmt::format("Fatal gateway error: {}", close_msg), close_code);
	}
	/* Reset the zlib inflate stream */
	if (Z_OK != inflateReset(&m_inflate_stream))
		throw std::runtime_error(m_inflate_stream.msg);
}
/* ================================================================================================================== */
cGatewayInfo
cGateway::implementation::get_gateway_info() {
	return cGatewayInfo{ DiscordGet("/gateway/bot").Wait() };
}
/* ================================================================================================================== */
void
cGateway::implementation::Run() {
	using namespace std::chrono_literals;
	/* Start the gateway loop */
	for (;;) {
		try {
			/* Start a new session */
			if (m_resume_gateway_url.empty())
				run_session(get_gateway_info().GetUrl());
			else
				run_session(m_resume_gateway_url);
		}
		catch (const xGatewayError& e) {
			/* Exit after a fatal gateway error that doesn't permit reconnecting */
			cUtils::PrintErr("{}", e.what());
			cUtils::PrintLog("Exiting...");
			break;
		}
		catch (...) {
			/* If anything happens, wait 5 seconds and try again */
			for (int tick = 5; tick > 0; --tick) {
				cUtils::PrintErr<'\r'>("Connection error. Retrying in {}s", tick);
				std::this_thread::sleep_for(1s);
			}
			cUtils::PrintErr("Connection error. Retrying...   ");
			std::this_thread::sleep_for(1s);
		}
	}
}