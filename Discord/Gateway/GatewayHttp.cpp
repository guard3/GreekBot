#include "GatewayImpl.h"
#include "json.h"
#include <tuple>

class cGateway::implementation::discord_request final {
private:
	cGateway::implementation* m_parent;

	asio::ip::tcp::resolver m_resolver;
	beast::ssl_stream<beast::tcp_stream> m_stream;
	beast::http::request<beast::http::string_body> m_request;
	beast::http::response<beast::http::string_body> m_response;

	std::exception_ptr m_except;
	std::tuple<beast::http::status, json::value> m_result;

public:
	discord_request(cGateway::implementation* this_, beast::http::verb method, const std::string& target, const json::object* obj, const tHttpFields& fields) :
		m_parent(this_),
		m_resolver(this_->m_http_ioc),
		m_stream(this_->m_http_ioc, this_->m_ctx),
		m_request(method, DISCORD_API_ENDPOINT + target, 11) {
		/* Set http fields */
		for (auto& f : fields)
			m_request.set(f.GetName(), f.GetValue());
		/* Also set the required ones */
		m_request.set(beast::http::field::host, DISCORD_API_HOST);
		m_request.set(beast::http::field::user_agent, "GreekBot");
		m_request.set(beast::http::field::authorization, this_->m_http_auth);
		if (obj) {
			m_request.set(beast::http::field::content_type, "application/json");
			m_request.body() = json::serialize(*obj);
		}
		m_request.prepare_payload();
	}

	bool await_ready() noexcept { return false; }
	void await_suspend(std::coroutine_handle<> h) {
		m_resolver.async_resolve(DISCORD_API_HOST, "https", [this, h](const beast::error_code& ec, asio::ip::tcp::resolver::results_type results) {
			if (ec) {
				m_except = std::make_exception_ptr(beast::system_error(ec));
				h();
				return;
			}
			beast::get_lowest_layer(m_stream).async_connect(results, [this, h](const beast::error_code& ec, asio::ip::tcp::endpoint ep) {
				if (ec) {
					m_except = std::make_exception_ptr(beast::system_error(ec));
					h();
					return;
				}
				m_stream.async_handshake(asio::ssl::stream_base::client, [this, h](const beast::error_code& ec) {
					if (ec) {
						m_except = std::make_exception_ptr(beast::system_error(ec));
						h();
						return;
					}
					beast::http::async_write(m_stream, m_request, [this, h](const beast::error_code& ec, size_t bytes_written) {
						if (ec) {
							m_except = std::make_exception_ptr(beast::system_error(ec));
							h();
							return;
						}
						beast::http::async_read(m_stream, m_parent->m_http_buffer, m_response, [this, h](beast::error_code ec, size_t bytes_read) {
							try {
								m_parent->m_http_buffer.consume(bytes_read);
								if (ec) throw beast::system_error(ec);
								m_stream.shutdown(ec);
								m_result = { m_response.result(), m_response[beast::http::field::content_type] == "application/json" ? json::parse(m_response.body()) : json::value() };
							}
							catch (...) {
								m_except = std::current_exception();
							}
							h();
						});
					});
				});
			});
		});
	}
	std::tuple<beast::http::status, json::value> await_resume() {
		/* If an error is set, throw an exception */
		if (m_except) std::rethrow_exception(m_except);
		return std::move(m_result);
	}
};

class cGateway::implementation::wait_on_event_thread : public std::suspend_always {
private:
	asio::steady_timer   m_timer;

public:
	wait_on_event_thread(cGateway::implementation* p, chrono::milliseconds r) : std::suspend_always(), m_timer(p->m_http_ioc, r) {}

	void await_suspend(std::coroutine_handle<> h) { m_timer.async_wait([h](const boost::system::error_code&) { h(); }); }
};

cTask<json::value>
cGateway::implementation::DiscordRequest(beast::http::verb m, const std::string& t, const json::object* o, const tHttpFields& f) {
	chrono::milliseconds retry_after;
	try {
		co_return co_await DiscordRequestNoRetry(m, t, o, f);
	}
	catch (const xRateLimitError& e) {
		retry_after = e.retry_after();
	}
	co_await wait_on_event_thread(this, retry_after);
	co_return co_await DiscordRequest(m, t, o, f);
}

cTask<json::value>
cGateway::implementation::DiscordRequestNoRetry(beast::http::verb m, const std::string& t, const json::object* o, const tHttpFields& f) {
	/* Perform the http request */
	beast::http::status status;
	json::value result;
	std::tie(status, result) = co_await discord_request(this, m, t, o, f);
	/* If request was successful, return received json value */
	if (beast::http::to_status_class(status) == beast::http::status_class::successful)
		co_return result;
	/* Otherwise, throw an appropriate exception */
	if (status == beast::http::status::too_many_requests)
		throw xRateLimitError(result);
	throw xDiscordError(result);
}
/* ================================================================================================================== */
cTask<>
cGateway::implementation::WaitOnEventThread(chrono::milliseconds duration) {
	co_await wait_on_event_thread(this, duration);
}

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
			json::value result = co_await m_parent->DiscordGet(cUtils::Format("/guilds/%s/members?limit=1000&after=%s", guild_id.ToString(), last_id.ToString()));
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
		json::value result = co_await m_parent->DiscordGet(cUtils::Format("/guilds/%s/members/search?limit=1000&query=%s", guild_id.ToString(), query));
		for (auto& v : result.as_array())
			co_yield cMember(v);
	}
	co_return;
}