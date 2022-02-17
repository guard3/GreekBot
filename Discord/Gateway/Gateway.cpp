/* Includes for Beast and Asio */
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio.hpp>
namespace beast = boost::beast;
namespace asio  = boost::asio;

#include <deque>
#include "Gateway.h"
#include "GatewayInfo.h"
#include "Event.h"
#include <iostream>
#include <zlib.h>

/* Gateway event/command opcodes */
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

/* A class with all asio and beast related objects to avoid including boost everywhere */
class cGatewaySession final {
private:
	asio::ssl::context ctx; // The SSL context

public:
	asio::io_context         ioc;    // The IO context
	asio::io_context::strand strand; // The strand for all async handlers

	beast::flat_buffer      buffer;    // A buffer for reading from the websocket
	std::deque<std::string> msg_queue; // A queue with all pending messages to be sent
	beast::websocket::stream<beast::ssl_stream<beast::tcp_stream>> ws; // The websocket stream

	z_stream inflate_stream;   // The zlib inflate stream for decompressing gateway payloads
	char inflate_buffer[4096]; // The buffer for storing decompressed data

	cGatewaySession() : ctx(asio::ssl::context::tlsv13_client), strand(ioc), ws(ioc, ctx) {
		memset(&inflate_stream, 0, sizeof(inflate_stream));
		if (Z_OK != inflateInit(&inflate_stream))
			throw std::runtime_error("Could not initialize inflate stream");
	}
	cGatewaySession(const cGatewaySession&) = delete;
	cGatewaySession(cGatewaySession&&) = delete;
	~cGatewaySession() { inflateEnd(&inflate_stream); }

	cGatewaySession& operator=(cGatewaySession) = delete;
};

cGateway::cGateway(const char *token, eIntent intents) : m_http_auth(cUtils::Format("Bot %s", token)), m_intents(intents), m_last_sequence(0), m_heartbeat_exit(false), m_heartbeat_ack(false), m_session(nullptr), m_close_code(-1), m_json_parser(&m_mr) {}

cGateway::~cGateway() { delete m_session; }

void
cGateway::send(std::string msg) {
	/* Make sure we're running on one strand */
	if (!m_session->strand.running_in_this_thread())
		return asio::post(m_session->strand, [this, s = std::move(msg)] { send(s); });
	/* Push the new message at the end of the message queue */
	m_session->msg_queue.push_back(std::move(msg));
	/* If there's no other message in the queue, start the asynchronous send operation */
	if (m_session->msg_queue.size() == 1)
		m_session->ws.async_write(asio::buffer(m_session->msg_queue.front()), asio::bind_executor(m_session->strand, [this](beast::error_code ec, size_t size) { on_write(ec, size); }));
}

void
cGateway::on_read(beast::error_code ec, size_t size) {
	/* If an error occurred, return and let the websocket stream close */
	if (ec)
		return;
	try {
		/* Check for Z_SYNC_FLUSH suffix and decompress if necessary */
		char* in = (char*)m_session->buffer.data().data();
		if (size >= 4 ) {
			if (0 == memcmp(in + (size - 4), "\x00\x00\xFF\xFF", 4)) {
				m_session->inflate_stream.avail_in = size;
				m_session->inflate_stream.next_in = (Byte*)in;
				do {
					m_session->inflate_stream.avail_out = 4096;
					m_session->inflate_stream.next_out = (Byte*)m_session->inflate_buffer;
					switch (inflate(&m_session->inflate_stream, Z_NO_FLUSH)) {
						default:
							throw std::runtime_error(m_session->inflate_stream.msg);
						case Z_OK:
						case Z_STREAM_END:
							m_json_parser.write(m_session->inflate_buffer, 4096 - m_session->inflate_stream.avail_out);
					}
				} while (m_session->inflate_stream.avail_out == 0);
				goto LABEL_PARSED;
			}
		}
		/* If the payload isn't compressed, directly feed it into the json parser */
		m_json_parser.write(in, size);
	LABEL_PARSED:
		/* Consume all read bytes from the dynamic buffer */
		m_session->buffer.consume(size);
		/* Release the parsed json value */
		json::value v = m_json_parser.release();
		/* Start the next asynchronous read operation to keep listening for more events */
		m_session->ws.async_read(m_session->buffer, asio::bind_executor(m_session->strand, [this](beast::error_code ec, size_t size) { on_read(ec, size); }));
#ifdef GW_LOG_LVL_2
		cUtils::PrintLog((std::stringstream() << v).str().c_str());
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
				start_heartbeating(v.at("d").at("heartbeat_interval").as_int64());
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
	m_json_parser.reset(&m_mr);
}

void
cGateway::on_write(beast::error_code ec, size_t size) {
	/* If an error occurred, return and let the websocket stream close */
	if (ec)
		return;
	/* Pop the message that was just sent */
	m_session->msg_queue.pop_front();
	/* If the queue isn't clear, continue the asynchronous send operation */
	if (!m_session->msg_queue.empty())
		m_session->ws.async_write(asio::buffer(m_session->msg_queue.front()), asio::bind_executor(m_session->strand, [this](beast::error_code ec, size_t size) { on_write(ec, size); }));
}

void
cGateway::run_session(const char* url) {
	/* Resolve host */
	const char* host = strstr(url, "://");
	if (host)
		host += 3;
	else
		host = url;
	/* Run */
	try {
		/* Create a new session */
		m_session = new cGatewaySession();
		/* Look up the domain name */
		auto results = asio::ip::tcp::resolver(m_session->ioc).resolve(host, "https");
		/* Set a timeout and make a connection to the resolved IP address */
		beast::get_lowest_layer(m_session->ws).expires_after(std::chrono::seconds(30));
		auto ep = beast::get_lowest_layer(m_session->ws).connect(results);
		/* Set a timeout and perform an SSL handshake */
		beast::get_lowest_layer(m_session->ws).expires_after(std::chrono::seconds(30));
		m_session->ws.next_layer().handshake(asio::ssl::stream_base::client);
		/* Use the recommended timout period for the websocket stream */
		beast::get_lowest_layer(m_session->ws).expires_never();
		m_session->ws.set_option(beast::websocket::stream_base::timeout::suggested(beast::role_type::client));
		/* Append resolved port number to host name */
		std::string host_str = cUtils::Format("%s:%d", host, ep.port());
		/* Set HTTP header fields for the handshake */
		m_session->ws.set_option(beast::websocket::stream_base::decorator([&](beast::websocket::request_type& r) {
			r.set(beast::http::field::host, host_str);
			r.set(beast::http::field::user_agent, "GreekBot");
		}));
		/* Perform websocket handshake */
		m_session->ws.handshake(host_str, cUtils::Format("/?v=%d&encoding=json&compress=zlib-stream", DISCORD_API_VERSION));
		/* Start the asynchronous read operation */
		m_session->ws.async_read(m_session->buffer, asio::bind_executor(m_session->strand, [this](beast::error_code ec, size_t size) { on_read(ec, size); }));
		m_session->ioc.run();
		/* When the websocket stream closes, save the reason */
		m_close_code = m_session->ws.reason().code;
		m_close_msg  = m_session->ws.reason().reason.c_str();
	}
	catch (const std::exception& e) {
		m_close_code = -1;
		m_close_msg  = e.what();
	}
	catch (...) {
		m_close_code = -1;
		m_close_msg = "An error occurred";
	}
	/* Stop heartbeating */
	stop_heartbeating();
	/* Delete session */
	delete m_session;
	m_session = nullptr;
}

uchGatewayInfo
cGateway::get_gateway_info(cDiscordError& error) {
	try {
		/* The JSON parser */
		json::monotonic_resource mr;
		json::stream_parser p(&mr);
		/* The http request response */
		std::string http_response;
		/* Return gateway info if http request is successful */
		if (200 == cDiscord::GetHttpsRequest(DISCORD_API_GATEWAY_BOT, GetHttpAuthorization(), http_response, error)) {
			p.write(http_response);
			return cHandle::MakeUnique<cGatewayInfo>(p.release());
		}
	}
	catch (...) {
		error = cDiscordError::MakeError<DISCORD_ERROR_GENERIC>();
	}
	return {};
}

void
cGateway::Run() {
	cDiscordError e;
	for (uchGatewayInfo g;;) {
		/* Get gateway info - retry with a timeout if any error occurs */
		for (int timeout = 5, tick; ; timeout += 5) {
			g = get_gateway_info(e);
			if (!e) break;
			switch (timeout) {
				case 20:
					fputc('\n', stderr);
					goto LABEL_RETURN;
				case 5:
					cUtils::PrintErr("Couldn't retrieve gateway info");
				default:
					if (e.GetType() == DISCORD_ERROR_HTTP) {
						cUtils::PrintErr(e.Get()->GetMessage());
						cUtils::PrintErr("Error code: %d", e.Get()->GetCode());
						goto LABEL_RETURN;
					}
					tick = timeout;
					do {
						fprintf(stderr, "[ERR] Retrying in %ds \r", tick);
						std::this_thread::sleep_for(std::chrono::seconds(1));
					} while (tick--);
			}
		}
		/* Connect to the gateway and run for a session */
		run_session(g->GetUrl());
		/* Handle session close codes */
		switch (const char* fmt; m_close_code) {
			case -1:
				fmt = "Error establishing connection: %s";
				goto LABEL_MSG;
			case 4004:
			case 4010:
			case 4011:
			case 4012:
			case 4013:
			case 4014:
				fmt = "Closing connection: %s";
			LABEL_MSG:
				cUtils::PrintErr(fmt, m_close_msg.c_str());
				return;
		}
	}
LABEL_RETURN:
	cUtils::PrintLog("Exiting...");
}