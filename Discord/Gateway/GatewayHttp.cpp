#include "GatewayImpl.h"
#include "json.h"

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

class cGateway::implementation::wait_on_event_thread {
private:
	asio::io_context& m_ioc;
	chrono::time_point<chrono::steady_clock> m_target_time;
	std::exception_ptr m_except;

	void postpone(std::coroutine_handle<> h) {
		try {
			if (chrono::steady_clock::now() < m_target_time) {
				asio::post(m_ioc, [this, h]() { postpone(h); });
				return;
			}
		}
		catch (...) {
			m_except = std::current_exception();
		}
		h();
	}

public:
	wait_on_event_thread(cGateway::implementation* p, chrono::milliseconds r) : m_ioc(p->m_http_ioc), m_target_time(chrono::steady_clock::now() + r) {
		cUtils::PrintLog("Rate limited! Waiting for %dms", r);
	}

	bool await_ready() noexcept { return false; }
	void await_suspend(std::coroutine_handle<> h) { asio::post(m_ioc, [this, h]() { postpone(h); }); }
	void await_resume() { if (m_except) std::rethrow_exception(m_except); }
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