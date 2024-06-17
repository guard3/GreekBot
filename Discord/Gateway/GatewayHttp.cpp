#include "Exception.h"
#include "GatewayImpl.h"
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
	/* First make sure that we are on the http thread */
	co_await ResumeOnEventThread();
	/* Since we're about to send a request, cancel the timeout timer */
	m_http_timer.cancel();
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
	co_await *this;
	/* Process response */
	auto status = m_response.result();
	auto result = m_response[beast::http::field::content_type] == "application/json" ? json::parse(m_response.body()) : json::value();
	if (beast::http::to_status_class(status) == beast::http::status_class::successful)
		co_return result;
	/* Otherwise, throw an appropriate exception */
	detail::throw_discord_exception(m_response.result_int(), result);
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