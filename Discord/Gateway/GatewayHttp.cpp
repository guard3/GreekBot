#include "GatewayImpl.h"
#include "json.h"

class cGateway::implementation::discord_request final {
private:
	cGateway::implementation* m_parent;
	/* Return value */
	std::tuple<beast::http::status, json::value> m_result;
	/* Necessary references */
	//asio::io_context& m_ioc;
	//asio::ssl::context& m_ctx;
	//beast::flat_buffer& m_buffer;
	asio::ip::tcp::resolver m_resolver;
	beast::ssl_stream<beast::tcp_stream> m_stream;
	beast::http::request<beast::http::string_body> m_request;
	beast::http::response<beast::http::string_body> m_response;
	/* Error code in case we need to throw an exception */
	beast::error_code m_ec;

public:
	discord_request(cGateway::implementation* this_, beast::http::verb method, const std::string& target, const json::object* obj, std::vector<cHttpField> fields) :
		//m_ioc(ioc),
		//m_ctx(ctx),
		//m_buffer(buff),
		m_parent(this_),
		m_resolver(this_->m_http_ioc),
		m_stream(this_->m_http_ioc, this_->m_ctx),
		m_request(method, DISCORD_API_ENDPOINT + target, 11) {
		/* Insert the rest */
		for (const cHttpField& f : fields)
			m_request.set(f.GetName(), f.GetValue());
		/* Insert the necessary fields here */
		m_request.set(beast::http::field::host, DISCORD_API_HOST);
		m_request.set(beast::http::field::user_agent, "GreekBot");
		m_request.set(beast::http::field::authorization, this_->m_http_auth);
		m_request.set(beast::http::field::accept, "application/json");
		if (obj) {
			m_request.set(beast::http::field::content_type, "application/json");
			m_request.body() = json::serialize(*obj);
		}
		m_request.prepare_payload();
	}

	bool await_ready() noexcept { return false; }
	void await_suspend(std::coroutine_handle<> handle) {
		m_resolver.async_resolve(DISCORD_API_HOST, "https", [this, handle](const beast::error_code& ec, asio::ip::tcp::resolver::results_type results) {
			if (ec) {
				m_ec = ec;
				handle();
				return;
			}
			beast::get_lowest_layer(m_stream).async_connect(results, [this, handle](const beast::error_code& ec, asio::ip::tcp::endpoint ep) {
				if (ec) {
					m_ec = ec;
					handle();
					return;
				}
				m_stream.async_handshake(asio::ssl::stream_base::client, [this, handle](const beast::error_code& ec) {
					if (ec) {
						m_ec = ec;
						handle();
						return;
					}
					beast::http::async_write(m_stream, m_request, [this, handle](const beast::error_code& ec, size_t bytes_written) {
						if (ec) {
							m_ec = ec;
							handle();
							return;
						}
						beast::http::async_read(m_stream, m_parent->m_http_buffer, m_response, [this, handle](beast::error_code ec, size_t bytes_read) {
							m_parent->m_http_buffer.consume(bytes_read);
							if (ec)
								m_ec = ec;
							else {
								m_result = { m_response.result(), m_response[beast::http::field::content_type] == "application/json" ? json::parse(m_response.body()) : json::object() };
								m_stream.shutdown(ec);
							}
							handle();
						});
					});
				});
			});
		});
	}
	std::tuple<beast::http::status, json::value> await_resume() {
		/* If an error is set, throw an exception */
		if (m_ec) throw xSystemError(m_ec.what(), m_ec.value());
		return std::move(m_result);
	}
};

class cGateway::implementation::retry_discord_request {
private:
	cGateway::implementation* m_parent;

	beast::http::verb m_method;
	const std::string& m_target;
	const json::object* m_object;
	const std::vector<cHttpField>& m_fields;

	std::chrono::time_point<std::chrono::steady_clock> m_target_time;
	uhHandle<cTask<json::value>> m_result;
	std::exception_ptr m_except;

public:
	retry_discord_request(cGateway::implementation* this_, double w, beast::http::verb m, const std::string& t, const json::object* o, const std::vector<cHttpField>& f) :
		m_parent(this_),
		m_method(m),
		m_target(t),
		m_object(o),
		m_fields(f),
		m_target_time(std::chrono::steady_clock::now() + std::chrono::milliseconds((int64_t)(w * 1000.0))) {
		cUtils::PrintLog("Rate limited! Waiting for %dms", (int)(w * 1000.0));
	}

	bool await_ready() noexcept { return false; }
	bool await_suspend(std::coroutine_handle<> h) noexcept {
		try {
			/* Postpone the next request until enough time has passed */
			if (std::chrono::steady_clock::now() < m_target_time) {
				asio::post(m_parent->m_http_ioc, [this, h]() { await_suspend(h); });
				return true;
			}
			/* Start and return the request task */
			m_result = cHandle::MakeUnique<cTask<json::value>>(m_parent->DiscordRequest(m_method, m_target, m_object, m_fields));
		}
		catch (...) {
			m_except = std::current_exception();
			h();
		}
		return false;
	}
	cTask<json::value> await_resume() {
		if (m_except) std::rethrow_exception(m_except);
		return std::move(*m_result);
	}
};

cTask<json::value>
cGateway::implementation::DiscordRequest(beast::http::verb m, const std::string& t, const json::object* o, const std::vector<cHttpField>& f) {
	/* Perform the http request */
	beast::http::status status;
	json::value result;
	std::tie(status, result) = co_await discord_request(this, m, t, o, f);
	/* If request was successful, return received json value */
	if (beast::http::to_status_class(status) == beast::http::status_class::successful)
		co_return result;
	/* If we're being rate limited, try again later */
	if (status == beast::http::status::too_many_requests)
		co_return co_await co_await retry_discord_request(this, result.at("retry_after").as_double(), m, t, o, f);
	/* If all else fails, throw */
	throw xDiscordError(result);
}

cTask<json::value>
cGateway::implementation::DiscordRequestNoRetry(beast::http::verb m, const std::string& t, const json::object* o, std::initializer_list<cHttpField> f) {
	/* Perform the http request */
	beast::http::status status;
	json::value result;
	std::tie(status, result) = co_await discord_request(this, m, t, o, f);
	/* If request was successful, return received json value */
	if (beast::http::to_status_class(status) == beast::http::status_class::successful)
		co_return result;
	/* Throw appropriate exception */
	if (status == beast::http::status::too_many_requests)
		throw std::runtime_error("RATE LIMITED");
	throw xDiscordError(result);
}