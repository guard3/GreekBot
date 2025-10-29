#ifndef DISCORD_GATEWAYIMPL_H
#define DISCORD_GATEWAYIMPL_H
#include "Application.h"
#include "Gateway.h"
#include "GuildMembersResult.h"
#include "ZlibError.h"
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
	/* Forward declaration of the websocket session type */
	struct websocket_session;
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
	/* Assignment */
	implementation& operator=(const implementation&) = delete;
	/* The parent gateway object through which events are handled */
	cGateway* m_parent;
	/* The contexts for asio */
	net::io_context   m_ioc;
	net::ssl::context m_ctx;
	/* Strands for synchronization */
	net::strand<net::io_context::executor_type> m_ws_strand;   // A strand for WebSocket operations
	net::strand<net::io_context::executor_type> m_http_strand; // A strand for HTTP operations
	/* Resolvers */
	net::ip::tcp::resolver m_ws_resolver;
	net::ip::tcp::resolver m_http_resolver;
	std::weak_ptr<websocket_session> m_ws_session;
	bool                     m_ws_terminating{}; // Flags of pending operations and status of the websocket stream
	steady_clock::time_point m_error_started_at; // The time point of the first connection error; used for retrying
	beast::flat_buffer       m_http_buffer;      // A buffer for storing http responses
	std::unique_ptr<ssl_stream> m_http_stream;   // The SSL stream used for HTTP
	/* HTTP */
	struct http_entry {
		http_request&        request;
		http_response&      response;
		std::coroutine_handle<> coro;
	};
	bool                   m_http_terminate{}; // Whether HTTP requests should be cancelled upon session termination;
	std::deque<http_entry> m_http_queue;       // A queue of pending HTTP requests
	net::steady_timer      m_http_timer;       // The timer for scheduling HTTP stream shutdown after a period of inactivity
	std::exception_ptr     m_http_exception;   // The exception to be propagated after an error during HTTP operations
	/* Initial parameters for identifying */
	std::string m_http_auth; // The authorization parameter for HTTP requests 'Bot token'
	eIntent     m_intents;   // The gateway intents
	/* Session attributes */
	urls::url    m_cached_gateway_url; // The url used for initial connections
	urls::url    m_resume_gateway_url; // The url used for resuming connections
	std::string  m_session_id;         // The current session id, used for resuming; empty = no valid session
	std::int64_t m_last_sequence{};    // The last event sequence received, used for heartbeating; 0 = none received
	/* Request Guild Members stuff */
	struct guild_members_entry {
		std::deque<cGuildMembersChunk> chunks;
		std::coroutine_handle<> coro;
		std::exception_ptr except;
		std::chrono::steady_clock::time_point started_at;
	};
	std::uint64_t                 m_rgm_nonce{};   // The nonce of the payload
	std::exception_ptr            m_rgm_exception; // An exception in case all pending requests need to be canceled
	std::deque<std::coroutine_handle<>> m_pending; // The queue of pending requests for when the gateway is unavailable
	std::unordered_map<std::uint64_t, guild_members_entry> m_rgm_entries; // A map to hold and manage all responses
	/* Partial application object */
	std::optional<cApplication> m_application;

	/* Gateway commands */
	void resume(std::shared_ptr<websocket_session>);
	void identify(std::shared_ptr<websocket_session>);
	void heartbeat(std::shared_ptr<websocket_session>);
	void on_heartbeat();
	/* Websocket message queuing */
	void send(std::shared_ptr<websocket_session>, std::string);
	/* Beast/Asio async functions */
	void on_read(std::shared_ptr<websocket_session>, const sys::error_code&) noexcept;
	void on_write(std::shared_ptr<websocket_session>, const sys::error_code&) noexcept;
	void on_expire(std::shared_ptr<websocket_session>, const sys::error_code&);
	void on_close(std::shared_ptr<websocket_session>) noexcept;
	void restart(std::shared_ptr<websocket_session>) noexcept;
	void retry(std::exception_ptr) noexcept;
	/* A method that initiates the gateway connection */
	void run_session() noexcept;
	void run_context() noexcept;
	/* A method that's invoked for every gateway event */
	void process_event(std::shared_ptr<websocket_session>, const json::value&);
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
};
/* ================================================================================================================== */
struct cGateway::implementation::websocket_session {
	beast::flat_buffer buffer;     // A buffer for reading from the websocket
	std::deque<std::string> queue; // A queue with all pending messages to be sent
	websocket_stream stream;       // The websocket stream
	/* Heartbeating */
	net::steady_timer hb_timer; // The async timer for heartbeats
	milliseconds hb_interval{}; // The interval between heartbeats
	bool hb_ack = false;        // Is the heartbeat acknowledged?
	bool closing = false;       // Are we closing this session?
	/* Json parsing for gateway events */
	json::stream_parser parser; // The json parser
	/* Zlib stuff for decompressing websocket messages */
	z_stream inflate_stream{};
	Byte inflate_buffer[4096];

	websocket_session(auto& ioc, auto& ctx) : stream(ioc, ctx), hb_timer(ioc) {
		/* Initialize zlib inflate stream */
		switch (int res = inflateInit(&inflate_stream)) {
			case Z_OK:
				break;
			case Z_ERRNO:
				throw std::system_error(errno, std::generic_category(), "Cannot initialize zlib stream");
			default:
				throw std::system_error(res, zlib::error_category(), "Cannot initialize zlib stream");
		}
	}
	~websocket_session() { inflateEnd(&inflate_stream); }

	websocket_session(const websocket_session&) = delete;
	websocket_session& operator=(const websocket_session&) = delete;
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