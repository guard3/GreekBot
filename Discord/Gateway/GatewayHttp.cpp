#include "Exception.h"
#include "GatewayImpl.h"
#include "Utils.h"
/* ================================================================================================================== */
net::awaitable<void>
cGateway::implementation::http_coro() {
	using namespace std::chrono_literals;
	/* If the HTTP stream is null, initialize a new one */
	if (!m_http_stream) {
		auto results = co_await m_http_resolver.async_resolve(DISCORD_API_HOST, "https", net::use_awaitable);
		auto stream = std::make_unique<ssl_stream>(m_http_strand, m_ctx);
		/* Connect and perform the SSL handshake; 30s timeout */
		stream->next_layer().expires_after(30s);
		co_await stream->next_layer().async_connect(results, net::use_awaitable);
		co_await stream->async_handshake(ssl_stream::client, net::use_awaitable);
		/* Save the stream on success */
		m_http_stream = std::move(stream);
	}
	/* Since we're about to send a request, cancel the timeout timer */
	m_http_timer.cancel();
	/* Using the now open HTTP stream, send the first request in the queue */
	m_http_stream->next_layer().expires_after(30s);
	co_await beast::http::async_write(*m_http_stream, m_http_queue.front().request, net::use_awaitable);
	/* Read the response */
	m_http_buffer.clear();
	auto[ec, _] = co_await beast::http::async_read(*m_http_stream, m_http_buffer, m_http_queue.front().response, net::as_tuple(net::use_awaitable));
	if (ec == beast::http::error::end_of_stream) {
		/* If the stream was unexpectedly closed, destroy it and try sending the same request again */
		m_http_stream.reset();
		co_return co_await http_coro();
	} else if (ec) {
		/* In the case of any other error, throw */
		throw sys::system_error(ec);
	}
}
void
cGateway::implementation::http_start() {
	net::co_spawn(m_http_strand, http_coro(), [this](std::exception_ptr ex) noexcept {
		using namespace std::chrono_literals;
		/* Pop the entry that was just processed */
		auto[_, response, coro] = m_http_queue.front();
		m_http_queue.pop_front();
		/* Save the exception pointer, whether null or not */
		m_http_exception = ex;
		try {
			if (!m_http_terminate && !ex && response.keep_alive()) {
				/* If the response indicates Keep-Alive, schedule the stream to close */
				m_http_timer.expires_after(1min);
				m_http_timer.async_wait([this](const sys::error_code &ec) {
					/* If the timer expired and there are no pending requests, destroy the stream */
					if (auto p = m_http_stream.get(); !ec && m_http_queue.empty() && p) {
						p->next_layer().expires_after(30s);
						p->async_shutdown([_ = std::move(m_http_stream)](const sys::error_code&) {});
					}
				});
			} else if (auto p = m_http_stream.get()) {
				/* In any other case, we have to destroy the stream */
				p->next_layer().expires_after(30s);
				p->async_shutdown([_ = std::move(m_http_stream)](const sys::error_code&) {});
			}
			/* If the HTTP session should terminate, fail with a session reset error */
			if (m_http_terminate)
				throw xGatewaySessionResetError();
			/* Start processing the next request in the queue */
			if (!m_http_queue.empty())
				http_start();
			/* Resume the coroutine */
			coro.resume();
		} catch (...) {
			/* Resume the coroutine with the original exception... */
			coro.resume();
			/* ...but then force the new exception to any pending request */
			m_http_exception = std::current_exception();
			for (auto[_1, _2, c] : m_http_queue)
				c.resume();
			m_http_queue.clear();
		}
	});
}
/* ================================================================================================================== */
cTask<json::value>
cGateway::implementation::DiscordRequest(beast::http::verb m, std::string_view t, const json::object* o, std::span<const cHttpField> f) {
	milliseconds retry_after;
	try {
		co_return co_await DiscordRequestNoRetry(m, t, o, f);
	}
	catch (const xRateLimitError& e) {
		retry_after = e.retry_after();
	}
	co_await WaitOnEventStrand(retry_after);
	co_return co_await DiscordRequest(m, t, o, f);
}
cTask<json::value>
cGateway::implementation::DiscordRequestNoRetry(beast::http::verb method, std::string_view target, const json::object* obj, std::span<const cHttpField> fields) {
	/* Create a new request object */
	http_request request(method, std::format(DISCORD_API_ENDPOINT "{}", target), 11);
	/* Set the necessary fields */
	for (auto&[name, value] : fields)
		request.set(name, value);
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
	/* The response object; this is where the HTTP response is saved from the awaitable */
	http_response response;
	/* Check that the current session isn't terminating */
	co_await ResumeOnEventStrand();
	if (m_http_terminate)
		throw xGatewaySessionResetError();
	/* Save the request and response references to the end of the request queue */
	m_http_queue.emplace_back(request, response);
	/* Start the async operation of sending the request */
	co_await [this] {
		struct _ {
			implementation* const self;
			constexpr bool await_ready() const noexcept { return false; }
			void await_suspend(std::coroutine_handle<> h) const {
				/* Save the coroutine handle to the entry we just added */
				self->m_http_queue.back().coro = h;
				/* If there's more than one request in the queue, just skip; it will be processed later */
				if (self->m_http_queue.size() == 1) try {
					self->http_start();
				} catch (...) {
					self->m_http_queue.pop_back();
					throw;
				}
			}
			void await_resume() const {
				if (self->m_http_exception)
					std::rethrow_exception(self->m_http_exception);
			}
		}; return _{ this };
	}();
	/* Process response */
	auto status = response.result();
	auto result = response[beast::http::field::content_type] == "application/json" ? json::parse(response.body()) : json::value();
	/* If the status indicates success, return result */
	if (beast::http::to_status_class(status) == beast::http::status_class::successful)
		co_return result;
	/* If the status indicates rate limiting, throw a rate limit exception */
	const json::object* pObj = result.if_object();
	/* If the HTTP status is 'too many requests', throw a special exception */
	if (status == beast::http::status::too_many_requests)
		throw xRateLimitError(response.reason(), pObj);
	// TODO: retry on error 502
	/* Otherwise throw a discord exception with the http error mixed in */
	try {
		throw xHttpError(static_cast<eHttpStatus>(status), response.reason());
	} catch (...) {
		eDiscordError ec = eDiscordError::GeneralError;
		if (const json::value* pValue; pObj && (pValue = pObj->if_contains("code"))) {
			/* Ignore code 0 because it's reserved for std::error_code to show success */
			if (auto code = pValue->to_number<std::underlying_type_t<eDiscordError>>(); code != 0)
				ec = static_cast<eDiscordError>(code);
			if ((pValue = pObj->if_contains("message")))
				std::throw_with_nested(xDiscordError(ec, pValue->as_string().c_str(), pObj->if_contains("errors")));
		}
		std::throw_with_nested(xDiscordError(ec));
	}
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