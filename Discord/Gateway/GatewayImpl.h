#ifndef DISCORD_GATEWAYIMPL_H
#define DISCORD_GATEWAYIMPL_H
#include "Application.h"
#include "Gateway.h"
#include "GuildMembersResult.h"
#include <deque>
#include <thread>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/json.hpp>
#include <boost/url.hpp>
#include <zlib.h>
/* ========= Namespace aliases ====================================================================================== */
namespace beast = boost::beast;
namespace json  = boost::json;
namespace net   = boost::asio;
namespace sys   = boost::system;
namespace urls  = boost::urls;
/* ========== Separate cGateway implementation class to avoid including boost everywhere ============================ */
struct cGateway::implementation final {
	/* Aliases for the stream types used for http and websocket with SSL support */
	using ssl_stream = net::ssl::stream<beast::tcp_stream>;
	using websocket_stream = beast::websocket::stream<ssl_stream>;
	using http_request = beast::http::request<beast::http::string_body>;
	using http_response = beast::http::response<beast::http::string_body>;
	using steady_clock = std::chrono::steady_clock;
	using milliseconds = std::chrono::milliseconds;
	/* Constructors */
	implementation(cGateway*, std::string_view, eIntent);
	implementation(const implementation&) = delete;
	~implementation();
	/* Assignment */
	implementation& operator=(const implementation&) = delete;
	/* The parent gateway object through which events are handled */
	cGateway* m_parent;
	/* The contexts for asio */
	net::io_context   m_ioc;
	net::ssl::context m_ctx;
	/* Strands for synchronisation */
	net::strand<net::io_context::executor_type> m_ws_strand;   // A strand for WebSocket operations
	net::strand<net::io_context::executor_type> m_http_strand; // A strand for HTTP operations
	/* Resolvers */
	net::ip::tcp::resolver   m_resolver;
	int                      m_async_status;     // Flags of pending operations and status of the websocket stream
	steady_clock::time_point m_error_started_at; // The time point of the first connection error; used for retrying
	beast::flat_buffer       m_buffer;           // A buffer for reading from the websocket
	beast::flat_buffer       m_http_buffer;      // A buffer for storing http responses
	std::deque<std::string>  m_queue;            // A queue with all pending messages to be sent
	std::unique_ptr<websocket_stream> m_ws;      // The websocket stream
	std::unique_ptr<ssl_stream> m_http_stream;   // The SSL stream used for HTTP
	/* HTTP */
	struct http_entry {
		http_request&        request;
		http_response&      response;
		std::coroutine_handle<> coro;
	};
	bool                   m_http_terminate; // Whether HTTP requests should be cancelled upon session termination;
	std::deque<http_entry> m_http_queue;     // A queue of pending HTTP requests
	net::steady_timer      m_http_timer;     // The timer for scheduling HTTP stream shutdown after a period of inactivity
	std::exception_ptr     m_http_exception; // The exception to be propagated after an error during HTTP operations
	/* Initial parameters for identifying */
	std::string m_http_auth; // The authorization parameter for HTTP requests 'Bot token'
	eIntent     m_intents;   // The gateway intents
	/* Session attributes */
	urls::url    m_cached_gateway_url; // The url used for initial connections
	urls::url    m_resume_gateway_url; // The url used for resuming connections
	std::string  m_session_id;         // The current session id, used for resuming; empty = no valid session
	std::int64_t m_last_sequence;      // The last event sequence received, used for heartbeating; 0 = none received
	/* Heartbeating */
	net::steady_timer m_heartbeat_timer;    // The async timer for heartbeats
	milliseconds      m_heartbeat_interval; // The interval between heartbeats
	bool              m_heartbeat_ack;      // Is the heartbeat acknowledged?
	/* Json parsing for gateway events */
	json::stream_parser m_parser; // The json parser
	/* Zlib stuff for decompressing websocket messages */
	z_stream m_inflate_stream;
	Byte     m_inflate_buffer[4096];
	/* Request Guild Members stuff */
	struct guild_members_entry {
		std::deque<cGuildMembersChunk> chunks;
		std::coroutine_handle<> coro;
		std::exception_ptr except;
		std::chrono::steady_clock::time_point started_at;
	};
	std::uint64_t                 m_rgm_nonce = 0; // The nonce of the payload
	std::exception_ptr            m_rgm_exception; // An exception in case all pending requests need to be canceled
	std::deque<std::coroutine_handle<>> m_pending; // The queue of pending requests for when the gateway is unavailable
	std::unordered_map<std::uint64_t, guild_members_entry> m_rgm_entries; // A map to hold and manage all responses
	/* Partial application object */
	std::optional<cApplication> m_application;
	/* A buffer to hold exception messages for when allocations are unfavorable */
	char m_err_msg[256];
	std::byte m_mem_rc_buff[4096];

	/* Gateway commands */
	void resume();
	void identify();
	void heartbeat();
	void on_heartbeat();
	/* Websocket message queuing */
	void send(std::string);
	/* Beast/Asio async functions */
	void on_read(const sys::error_code&, std::size_t);
	void on_write(const sys::error_code&);
	void on_expire(const sys::error_code&);
	void on_close() noexcept;
	void restart() noexcept;
	void retry(std::exception_ptr) noexcept;
	/* A method that initiates the gateway connection */
	void run_session() noexcept;
	void run_context() noexcept;
	/* A method that's invoked for every gateway event */
	void process_event(const json::value&);
	/* Http functions */
	void http_start();
	net::awaitable<void> http_coro();
	/* Canceling guild member requests */
	void rgm_reset() noexcept;   // Cancel all pending requests when the session is reset
	void rgm_timeout() noexcept; // Cancel all pending requests that haven't been completed in a long time

	cTask<json::value> DiscordRequest(beast::http::verb method, std::string_view target, const json::object* obj, std::span<const cHttpField> fields);
	cTask<json::value> DiscordRequestNoRetry(beast::http::verb method, std::string_view target, const json::object* obj, std::span<const cHttpField> fields);

	cTask<json::value> DiscordGet   (std::string_view path,                          std::span<const cHttpField> fields = {});
	cTask<json::value> DiscordPost  (std::string_view path, const json::object& obj, std::span<const cHttpField> fields = {});
	cTask<json::value> DiscordPatch (std::string_view path, const json::object& obj, std::span<const cHttpField> fields = {});
	cTask<json::value> DiscordPut   (std::string_view path,                          std::span<const cHttpField> fields = {});
	cTask<json::value> DiscordPut   (std::string_view path, const json::object& obj, std::span<const cHttpField> fields = {});
	cTask<json::value> DiscordDelete(std::string_view path,                          std::span<const cHttpField> fields = {});

	cTask<json::value> DiscordPostNoRetry(std::string_view path, const json::object& obj, std::span<const cHttpField> fields = {});

	std::string_view GetHttpAuthorization() const noexcept { return m_http_auth; }
	std::string_view GetToken() const noexcept { return GetHttpAuthorization().substr(4); }

	struct strand_awaitable {
		net::strand<net::io_context::executor_type>& strand;
		bool await_ready() const noexcept { return strand.running_in_this_thread(); }
		void await_suspend(std::coroutine_handle<> h) const noexcept { net::post(strand, h); }
		void await_resume() const noexcept {}
	};

	strand_awaitable ResumeOnWebSocketStrand() noexcept { return { m_ws_strand }; }
	strand_awaitable ResumeOnEventStrand() noexcept { return { m_http_strand }; }
	auto WaitOnEventStrand(std::chrono::milliseconds duration) {
		struct awaitable {
			net::steady_timer timer;
			awaitable(net::strand<net::io_context::executor_type>& strand, std::chrono::milliseconds d): timer(strand, d) {}
			bool await_ready() noexcept { return false; }
			void await_suspend(std::coroutine_handle<> h) {
				timer.async_wait([h](const sys::error_code&) { h.resume(); });
			}
			void await_resume() noexcept {}
		};
		return awaitable(m_http_strand, duration);
	}

	cAsyncGenerator<cMember> RequestGuildMembers(const cSnowflake&, std::string_view);
	cAsyncGenerator<cMember> RequestGuildMembers(const cSnowflake&, std::span<const cSnowflake>);
	cAsyncGenerator<cMember> RequestGuildMembers(const cSnowflake&, std::string_view, std::span<const cSnowflake>);

	void Run();
	void RequestExit() noexcept;
};
/* ========== Make void a valid coroutine return type for cGateway::implementation member functions ================= */
template<typename... Args>
struct coroutine_traits_base {
	struct promise_type {
		cGateway::implementation& self;
		promise_type(cGateway::implementation& self, Args...) noexcept : self(self) {}

		constexpr void get_return_object() const noexcept {}
		constexpr auto initial_suspend() const noexcept { return std::suspend_never{}; }
		constexpr auto final_suspend() const noexcept { return std::suspend_never{}; }
		constexpr void return_void() const noexcept {}

		void unhandled_exception() const noexcept {
			net::post(self.m_ioc, [ex = std::current_exception()] { std::rethrow_exception(ex); });
		}
	};
};
template<typename... Args>
struct std::coroutine_traits<void, cGateway::implementation&, Args...> : coroutine_traits_base<Args...> {};
#endif /* DISCORD_GATEWAYIMPL_H */