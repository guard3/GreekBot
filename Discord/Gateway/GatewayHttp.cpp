#include "GatewayImpl.h"
#include "json.h"

class discord_http_request final {
private:
	/* Return value */
	std::tuple<beast::http::status, json::value> m_result;
	/* Necessary references */
	asio::io_context& m_ioc;
	asio::ssl::context& m_ctx;
	beast::flat_buffer& m_buffer;
	asio::ip::tcp::resolver m_resolver;
	beast::ssl_stream<beast::tcp_stream> m_stream;
	beast::http::request<beast::http::string_body> m_request;
	beast::http::response<beast::http::string_body> m_response;
	/* Error code in case we need to throw an exception */
	beast::error_code m_ec;

public:
	template<typename... Args>
	discord_http_request(beast::http::verb method, const std::string& target, const std::string& auth, const json::object* obj, std::initializer_list<cHttpField> fields, asio::io_context& ioc, asio::ssl::context& ctx, beast::flat_buffer& buff) :
		m_ioc(ioc),
		m_ctx(ctx),
		m_buffer(buff),
		m_resolver(m_ioc),
		m_stream(m_ioc, m_ctx),
		m_request(method, DISCORD_API_ENDPOINT + target, 11) {
		/* Insert the rest */
		for (const cHttpField& f : fields)
			m_request.set(f.GetName(), f.GetValue());
		/* Insert the necessary fields here */
		m_request.set(beast::http::field::host, DISCORD_API_HOST);
		m_request.set(beast::http::field::user_agent, "GreekBot");
		m_request.set(beast::http::field::authorization, auth);
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
						beast::http::async_read(m_stream, m_buffer, m_response, [this, handle](beast::error_code ec, size_t bytes_read) {
							m_buffer.consume(bytes_read);
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

cTask<json::value>
cGateway::implementation::DiscordRequest(beast::http::verb m, const std::string& t, const json::object* o, std::initializer_list<cHttpField> f) {
	/* Perform the http request */
	beast::http::status status;
	json::value result;
	try {
		std::tie(status, result) = co_await discord_http_request(m, t, m_http_auth, o, f, m_http_ioc, m_ctx, m_http_buffer);
		/* If request was successful, return received json value */
		if (beast::http::to_status_class(status) == beast::http::status_class::successful)
			co_return result;
	}
	catch (const boost::system::system_error& e) {
		/* An HTTP or JSON parsing error occurred */
		throw xSystemError(e.what(), e.code().value());
	}
	/* TODO: make this async */
	/* If we're being rate limited, wait and try again */
	if (status == beast::http::status::too_many_requests) {
		std::this_thread::sleep_for(std::chrono::milliseconds((uint64_t)(result.at("retry_after").as_double() * 1000.0)));
		co_return co_await DiscordRequest(m, t, o, f);
	}
	/* If all else fails, throw */
	throw xDiscordError(result);
}