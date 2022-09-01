#include "Http.h"

class http_request final {
private:
	/* Arguments */
	std::string m_host;
	/* Return value */
	std::tuple<unsigned int, std::string> m_result;
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
	http_request(beast::http::verb method, std::string&& host, std::string&& path, std::string&& content, std::initializer_list<cHttpField>&& fields, asio::io_context& ioc, asio::ssl::context& ctx, beast::flat_buffer& buff) :
		m_host(std::forward<std::string>(host)),
		m_ioc(ioc),
		m_ctx(ctx),
		m_buffer(buff),
		m_resolver(m_ioc),
		m_stream(m_ioc, m_ctx),
		m_request(method, std::forward<std::string>(path), 11) {
		/* Insert the necessary fields here */
		m_request.insert(beast::http::field::host, m_host);
		m_request.insert(beast::http::field::user_agent, "GreekBot");
		/* Insert the rest */
		for (const cHttpField& f : fields)
			m_request.insert(f.GetName(), f.GetValue());
		m_request.body() = std::forward<std::string>(content);
		m_request.prepare_payload();
	}

	bool await_ready() noexcept { return false; }
	void await_suspend(std::coroutine_handle<> handle) {
		m_resolver.async_resolve(std::move(m_host), "https", [this, handle](const beast::error_code& ec, asio::ip::tcp::resolver::results_type results) {
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
							if (ec) {
								m_ec = ec;
								handle();
								return;
							}
							m_result = { m_response.result_int(), std::move(m_response.body()) };
							m_stream.shutdown(ec);
							handle();
						});
					});
				});
			});
		});
	}
	std::tuple<unsigned int, std::string> await_resume() {
		/* If an error is set, throw an exception */
		if (m_ec) throw xSystemError(m_ec.what(), m_ec.value());
		return std::move(m_result);
	}
};

tHttpTask
cHttp::Get(std::string h, std::string t, tHttpFields f) {
	co_return co_await http_request(beast::http::verb::get, std::move(h), std::move(t), {}, std::move(f), m_ioc, m_ctx, m_buffer);
}
tHttpTask
cHttp::Post(std::string h, std::string t, std::string c, tHttpFields f) {
	co_return co_await http_request(beast::http::verb::post, std::move(h), std::move(t), std::move(c), std::move(f), m_ioc, m_ctx, m_buffer);
}
tHttpTask
cHttp::Patch(std::string h, std::string t, std::string c, tHttpFields f) {
	co_return co_await http_request(beast::http::verb::patch, std::move(h), std::move(t), std::move(c), std::move(f), m_ioc, m_ctx, m_buffer);
}
tHttpTask
cHttp::Put(std::string h, std::string t, std::string c, tHttpFields f) {
	co_return co_await http_request(beast::http::verb::put, std::move(h), std::move(t), std::move(c), std::move(f), m_ioc, m_ctx, m_buffer);
}
tHttpTask
cHttp::Delete(std::string h, std::string t, std::string c, tHttpFields f) {
	co_return co_await http_request(beast::http::verb::delete_, std::move(h), std::move(t), std::move(c), std::move(f), m_ioc, m_ctx, m_buffer);
}