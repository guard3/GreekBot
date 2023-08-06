#ifndef GREEKBOT_GATEWAYIMPL_H
#define GREEKBOT_GATEWAYIMPL_H
#include "Gateway.h"
#include "beast.h"
#include "json.h"
#include <deque>
#include <zlib.h>
#include <thread>
#include "GuildMembersResult.h"
#include "Application.h"

namespace chrono = std::chrono;

#define INFLATE_BUFFER_SIZE 4096

class cEvent;
class cGatewayInfo;

/* Gateway close error exception */
class xGatewayError : public xSystemError {
public:
	explicit xGatewayError(const std::string& what, int code = 0) : xSystemError(what, code) {}
	explicit xGatewayError(const char*        what, int code = 0) : xSystemError(what, code) {}
	xGatewayError(const xGatewayError&) = default;
	xGatewayError(xGatewayError&&)      = default;

	xGatewayError& operator=(const xGatewayError&) = default;
	xGatewayError& operator=(xGatewayError&&)      = default;
};

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

/* Separate cGateway implementation class to avoid including boost everywhere */
class cGateway::implementation final {
private:
	/* The parent gateway object through which events are handled */
	cGateway* m_parent;
	/* The contexts for asio */
	asio::io_context   m_ws_ioc;   // IO context for websocket
	asio::io_context   m_http_ioc; // IO context for http
	asio::ssl::context m_ctx;      // SLL context
	/* Resolvers */
	asio::ip::tcp::resolver m_ws_resolver;
	asio::ip::tcp::resolver m_http_resolver;
	/* Work guard to prevent the http io_context from running out of work */
	asio::executor_work_guard<asio::io_context::executor_type> m_work;
	std::thread m_work_thread;

	beast::flat_buffer      m_buffer;      // A buffer for reading from the websocket
	beast::flat_buffer      m_http_buffer; // A buffer for storing http responses
	std::deque<std::string> m_queue;       // A queue with all pending messages to be sent
	uhHandle<beast::websocket::stream<beast::ssl_stream<beast::tcp_stream>>> m_ws; // The websocket stream

	/* Initial parameters for identifying */
	std::string m_http_auth; // The authorization parameter for HTTP requests 'Bot token'
	eIntent     m_intents;   // The gateway intents
	/* Session attributes */
	std::string m_resume_gateway_url; // The url used for resuming connections
	std::string m_session_id;         // The current session id, used for resuming; empty = no valid session
	int64_t     m_last_sequence;      // The last event sequence received, used for heartbeating; 0 = none received
	/* Heartbeating */
	asio::steady_timer   m_heartbeat_timer;    // The async timer for heartbeats
	chrono::milliseconds m_heartbeat_interval; // The interval between heartbeats
	bool                 m_heartbeat_ack;      // Is the heartbeat acknowledged?
	/* Json parsing for gateway events */
	json::monotonic_resource m_mr;     // A monotonic memory resource for json parsing
	json::stream_parser      m_parser; // The json parser
	/* Zlib stuff for decompressing websocket messages */
	z_stream m_inflate_stream;
	Byte     m_inflate_buffer[INFLATE_BUFFER_SIZE];
	/* Request Guild Members stuff */
	std::string m_rgm_payload;   // The gateway payload to be sent
	uint64_t    m_rgm_nonce = 0; // The nonce of the payload
	std::unordered_map<uint64_t, cGuildMembersResult> m_rgm_map; // A map to hold and manage all responses
	/* Partial application object */
	std::optional<cApplication> m_application;

	/* Gateway commands */
	void resume();
	void identify();
	void heartbeat();
	/* Websocket message queuing */
	void send(std::string);
	/* Beast/Asio async functions */
	void on_read(const beast::error_code&, size_t);
	void on_write(const beast::error_code&, size_t);
	void on_expire(const beast::error_code&);
	/* A method that initiates the gateway connection */
	void run_session(const std::string& url);
	cGatewayInfo get_gateway_info();
	/* A method that's invoked for every gateway event */
	void on_event(cEvent);
	/* Gateway command functions */
	bool await_ready() { return false; }
	void await_suspend(std::coroutine_handle<> h) {
		/* Save the coroutine handle to resume after all members have been received */
		m_rgm_map[m_rgm_nonce++].Fill(h);
		/* Send the payload */
		send(std::move(m_rgm_payload));
	}
	void await_resume() {}

	void rgm_reset() {
		m_rgm_payload.clear();
		m_rgm_map.clear();
		m_rgm_nonce = 0;
	}

	class discord_request;
	class wait_on_event_thread;

public:
	implementation(cGateway*, std::string_view, eIntent);
	implementation(const implementation&) = delete;
	~implementation();

	implementation& operator=(const implementation&) = delete;

	cTask<json::value> DiscordRequest(beast::http::verb method, const std::string& target, const json::object* obj, const tHttpFields& fields);
	cTask<json::value> DiscordRequestNoRetry(beast::http::verb method, const std::string& target, const json::object* obj, const tHttpFields& fields);

	std::string_view GetHttpAuthorization() const noexcept { return m_http_auth; }
	std::string_view GetToken() const noexcept { return GetHttpAuthorization().substr(4); }

	cTask<> ResumeOnEventThread();
	cTask<> WaitOnEventThread(chrono::milliseconds);
	cAsyncGenerator<cMember> get_guild_members(const cSnowflake& guild_id, const std::string& query, const std::vector<cSnowflake>& user_ids);

	void Run();
};
#endif /* GREEKBOT_GATEWAYIMPL_H */