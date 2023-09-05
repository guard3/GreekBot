#include "GatewayImpl.h"
#include "json.h"
#include <tuple>
#include <fmt/format.h>
/* ================================================================================================================== */
void
cGateway::implementation::http_resolve() {
	using namespace std::chrono_literals;
	m_resolver.async_resolve(DISCORD_API_HOST, "https", [this](const beast::error_code& ec, const asio::ip::tcp::resolver::results_type& results) {
		try {
			if (ec) throw std::system_error(ec);
			/* Create a new http stream */
			m_http_buffer.clear();
			m_http_stream = std::make_unique<beast::ssl_stream<beast::tcp_stream>>(m_http_strand, m_ctx);
			/* Connect to one of the resolved endpoints */
			m_http_stream->next_layer().expires_after(30s);
			m_http_stream->next_layer().async_connect(results, [this](const beast::error_code& ec, const asio::ip::tcp::endpoint&) {
				try {
					if (ec) throw std::system_error(ec);
					m_http_stream->async_handshake(asio::ssl::stream_base::client, [this](const beast::error_code& ec) {
						try {
							if (ec) throw std::system_error(ec);
							http_write();
						}
						catch (...) {
							m_except = std::current_exception();
							http_shutdown();
						}
					});
				}
				catch (...) {
					m_except = std::current_exception();
					http_shutdown();
				}
			});
		}
		catch (...) {
			m_except = std::current_exception();
			http_shutdown();
		}
	});
}
void
cGateway::implementation::http_write() {
	using namespace std::chrono_literals;
	m_http_stream->next_layer().expires_after(30s);
	beast::http::async_write(*m_http_stream, m_request_queue.front().request, [this](const beast::error_code& ec, size_t) {
		try {
			if (ec) throw std::system_error(ec);
			/* Reset the response before reading */
			m_response = {};
			/* Start reading the response */
			beast::http::async_read(*m_http_stream, m_http_buffer, m_response, [this](const beast::error_code& ec, size_t) {
				/* If connection was unexpectedly closed, retry */
				if (ec == beast::http::error::end_of_stream) {
					http_resolve();
					return;
				}
				/* If there was any other error, fail */
				try {
					if (ec) throw std::system_error(ec);
				}
				catch (...) {
					m_except = std::current_exception();
					http_shutdown_ssl();
					return;
				}
				/* If the server doesn't keep the connection alive, gracefully close the connection */
				if (!m_response.keep_alive()) {
					http_shutdown_ssl();
					return;
				}
				/* Save the coroutine handle before popping the processed request from the queue */
				std::coroutine_handle<> coro = m_request_queue.front().coro;
				m_request_queue.pop_front();
				if (m_request_queue.empty()) try {
					/* If there are no more requests, schedule the connection to close after 1 minute */
					m_http_timer.expires_after(1min);
					m_http_timer.async_wait([this](const boost::system::error_code &ec) {
						/* If the timer expired and there are no pending requests, close the connection */
						if (ec != asio::error::operation_aborted && m_request_queue.empty()) {
							auto stream = m_http_stream.get();
							stream->next_layer().expires_after(30s);
							stream->async_shutdown([_ = std::move(m_http_stream)](const beast::error_code &ec) {});
						}
					});
				}
				catch (...) {
					m_except = std::current_exception();
				}
				else {
					/* If there are pending requests, write the next one to the stream */
					http_write();
				}
				coro.resume();
			});
		}
		catch (...) {
			m_except = std::current_exception();
			http_shutdown_ssl();
		}
	});
}
void
cGateway::implementation::http_shutdown_ssl() {
	using namespace std::chrono_literals;
	/* Destroy the http stream */
	auto stream = m_http_stream.get();
	stream->next_layer().expires_after(30s);
	stream->async_shutdown([_ = std::move(m_http_stream)](const beast::error_code&){});
	/* Save the coroutine handle before popping the current entry from the queue */
	auto coro = m_request_queue.front().coro;
	m_request_queue.pop_front();
	/* Start processing the rest of the requests in the queue */
	if (!m_request_queue.empty())
		http_resolve();
	/* Resume the coroutine */
	coro.resume();
}
void
cGateway::implementation::http_shutdown() {
	/* Destroy the http stream */
	m_http_stream = nullptr;
	/* Save the coroutine handle before popping the current entry from the queue */
	std::coroutine_handle<> coro = m_request_queue.front().coro;
	m_request_queue.pop_front();
	/* Start processing the rest of the requests in the queue */
	if (!m_request_queue.empty())
		http_resolve();
	/* Resume the coroutine */
	coro.resume();
}
/* ================================================================================================================== */
cTask<json::value>
cGateway::implementation::DiscordRequest(beast::http::verb m, std::string_view t, const json::object* o, std::span<const cHttpField> f) {
	chrono::milliseconds retry_after;
	try {
		co_return co_await DiscordRequestNoRetry(m, t, o, f);
	}
	catch (const xRateLimitError& e) {
		retry_after = e.retry_after();
	}
	co_await WaitOnEventThread(retry_after);
	co_return co_await DiscordRequest(m, t, o, f);
}
cTask<json::value>
cGateway::implementation::DiscordRequestNoRetry(beast::http::verb method, std::string_view target, const json::object* obj, std::span<const cHttpField> fields) {
	/* Since we're about to send a request, cancel the timeout timer */
	m_http_timer.cancel();
	/* First make sure that we are on the http thread */
	co_await ResumeOnEventThread();
	/* Create a new request object and push it at the end of the queue */
	auto &request = m_request_queue.emplace_back(method, fmt::format(DISCORD_API_ENDPOINT "{}", target), 11).request;
	/* Set user http fields */
	for (auto &f: fields)
		request.set(f.GetName(), f.GetValue());
	/* Also set the required ones */
	request.set(beast::http::field::host, DISCORD_API_HOST);
	request.set(beast::http::field::user_agent, "GreekBot");
	request.set(beast::http::field::authorization, m_http_auth);
	if (obj) {
		request.set(beast::http::field::content_type, "application/json");
		request.body() = json::serialize(*obj);
	}
	/* Request that the connection be kept alive */
	request.keep_alive(true);
	request.prepare_payload();
	/* Start the async operation of sending the request */
	m_await_command = AWAIT_REQUEST;
	co_await *this;
	/* Process response */
	auto status = m_response.result();
	auto result = m_response[beast::http::field::content_type] == "application/json" ? json::parse(m_response.body()) : json::value();
	if (beast::http::to_status_class(status) == beast::http::status_class::successful)
		co_return result;
	/* Otherwise, throw an appropriate exception */
	if (status == beast::http::status::too_many_requests)
		throw xRateLimitError(result);
	throw xDiscordError(result);
}
/* ================================================================================================================== */
cTask<json::value>
cGateway::implementation::DiscordGet(std::string_view t, std::span<const cHttpField> f) {
	return DiscordRequest(beast::http::verb::get, t, nullptr, f);
}
cTask<json::value>
cGateway::implementation::DiscordPost(std::string_view t, const json::object& o, std::span<const cHttpField> f) {
	return DiscordRequest(beast::http::verb::post, t, &o, f);
}
cTask<json::value>
cGateway::implementation::DiscordPatch(std::string_view t, const json::object& o, std::span<const cHttpField> f) {
	return DiscordRequest(beast::http::verb::patch, t, &o, f);
}
cTask<json::value>
cGateway::implementation::DiscordPut(std::string_view t, std::span<const cHttpField> f) {
	return DiscordRequest(beast::http::verb::put, t, nullptr, f);
}
cTask<json::value>
cGateway::implementation::DiscordPut(std::string_view t, const json::object& o, std::span<const cHttpField> f) {
	return DiscordRequest(beast::http::verb::put, t, &o, f);
}
cTask<json::value>
cGateway::implementation::DiscordDelete(std::string_view t, std::span<const cHttpField> f) {
	return DiscordRequest(beast::http::verb::delete_, t, nullptr, f);
}
/* ================================================================================================================== */
cTask<json::value>
cGateway::implementation::DiscordPostNoRetry(std::string_view t, const json::object& o, std::span<const cHttpField> f) {
	return DiscordRequestNoRetry(beast::http::verb::post, t, &o, f);
}
/* ================================================================================================================== */
// TODO: make custom exceptions, preferably in Exceptions.h
// TODO: make getting all members from the gateway
cAsyncGenerator<cMember>
cGateway::implementation::get_guild_members(const cSnowflake& guild_id, const std::string& query, const std::vector<cSnowflake>& user_ids) {
	if (!user_ids.empty()) {
		/* Make sure we're running on the event thread */
		co_await ResumeOnEventThread();
		/* Save the current nonce to get the result later */
		auto nonce = m_rgm_nonce;
		/* Prepare the gateway payload */
		json::array a;
		a.reserve(user_ids.size());
		for (auto& s : user_ids)
			a.emplace_back(s.ToString());
		m_rgm_payload = json::serialize(json::object {
			{ "op", 8 },
			{ "d", json::object {
				{ "guild_id", guild_id.ToString() },
				{ "user_ids", std::move(a) },
				{ "nonce", std::to_string(nonce) }
			}}
		});
		/* Send the payload to the gateway and wait for the result to be available */
		m_await_command = AWAIT_GUILD_MEMBERS;
		co_await *this;
		/* Extract the result node */
		auto node = m_rgm_map.extract(nonce);
		/* If the map is empty, reset the nonce count */
		if (m_rgm_map.empty())
			m_rgm_nonce = 0;
		/* Make sure node is not empty for whatever reason */
		if (!node) throw std::runtime_error("Unexpected empty result");
		/* Return the results one by one */
		for (auto gen = node.mapped().Publish(); gen;)
			co_yield gen();
		co_return;
	}
	if (query.empty()) {
		/* If no query is specified, return all guild members, provided we have the appropriate intents */
		if (!(m_application->GetFlags() & (APP_FLAG_GATEWAY_GUILD_MEMBERS | APP_FLAG_GATEWAY_GUILD_MEMBERS_LIMITED)))
			throw std::runtime_error("Missing privileged intents");
		for (cSnowflake last_id = "0";;) {
			json::value result = co_await DiscordGet(fmt::format("/guilds/{}/members?limit=1000&after={}", guild_id, last_id));
			json::array& members = result.as_array();
			if (members.empty())
				break;
			for (auto i = members.begin(); i < members.end() - 1; ++i)
				co_yield cMember(*i);
			cMember last_member(members.back());
			last_id = last_member.GetUser()->GetId();
			co_yield std::move(last_member);
			if (members.size() < 1000)
				break;
		}
	}
	else {
		/* If there is a query, return matching guild members */
		json::value result = co_await DiscordGet(fmt::format("/guilds/{}/members/search?limit=1000&query={}", guild_id, query));
		for (auto& v : result.as_array())
			co_yield cMember(v);
	}
	co_return;
}