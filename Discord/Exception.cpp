#include "Exception.h"
#include "json.h"
/* ================================================================================================================== */
static std::shared_ptr<char[]> shared_copy(const char* str) {
	if (!str) return {};
	auto res = std::make_shared<char[]>(std::strlen(str) + 1);
	std::strcpy(res.get(), str);
	return res;
}
/* ================================================================================================================== */
xRateLimitError::xRateLimitError(unsigned status, const char* msg, std::chrono::milliseconds ra, bool global):
	xHTTPError(status, msg ? msg : "You are being rate limited."),
	m_retry_after(ra),
	m_global(global) {}
std::chrono::milliseconds
xRateLimitError::retry_after() const noexcept {
	return m_retry_after;
}
bool
xRateLimitError::global() const noexcept {
	return m_global;
}
/* ================================================================================================================== */
xDiscordError::xDiscordError(unsigned status, eDiscordError code, const char *msg, const char *err):
	xHTTPError(status, msg ? msg : "Discord error"),
	m_code(code),
	m_errors(shared_copy(err)) {}
const char*
xDiscordError::errors() const noexcept {
	return m_errors ? m_errors.get() : "{}";
}
eDiscordError
xDiscordError::code() const noexcept {
	return m_code;
}
/* ================================================================================================================== */
xUnknownWebhookError::xUnknownWebhookError(unsigned status, const char* msg, const char* err): xDiscordError(status, eDiscordError::UnknownWebhook, msg, err) {}
xInteractionAcknowledgedError::xInteractionAcknowledgedError(unsigned status, const char* msg, const char* err) : xDiscordError(status, eDiscordError::InteractionAcknowledged, msg, err) {}
xInvalidFormBodyError::xInvalidFormBodyError(unsigned status, const char* msg, const char* err) : xDiscordError(status, eDiscordError::InvalidFormBody, msg, err) {}
/* ================================================================================================================== */
xInteractionAcknowledgedError::xInteractionAcknowledgedError() : xInteractionAcknowledgedError(400, "Interaction has already been acknowledged.", nullptr) {}
xInvalidFormBodyError::xInvalidFormBodyError() : xInvalidFormBodyError(400, "Invalid Form Body", nullptr) {}
/* ================================================================================================================== */
[[noreturn]]
void detail::throw_rate_limit_exception(unsigned status, const boost::json::value& v) {
	using namespace std::chrono;
	/* Exception parameters */
	const char* message = nullptr;
	auto retry_after = duration<double>(0);
	bool global = false;
	/* If the input JSON value is an object, parse its contents */
	if (auto o = v.if_object()) {
		if (auto p = o->if_contains("message")) {
			if (auto s = p->if_string())
				message = s->c_str();
		}
		if (auto p = o->if_contains("retry_after")) {
			if (auto t = p->if_double())
				retry_after = duration<double>(*t);
		}
		if (auto p = o->if_contains("global")) {
			auto b = p->if_bool();
			global = b && *b;
		}
	}
	throw xRateLimitError(status, message, duration_cast<milliseconds>(retry_after), global);
}
[[noreturn]]
void detail::throw_discord_exception(unsigned status, const boost::json::value& v) {
	/* If the HTTP status is 'too many requests', throw appropriate exception */
	if (status == 429)
		throw_rate_limit_exception(status, v);
	/* Exception parameters */
	eDiscordError code = eDiscordError::GeneralError;
	const char* msg = nullptr;
	const char* err = nullptr;
	/* If the input JSON value is an object, parse its contents */
	if (auto o = v.if_object()) {
		if (auto p = o->if_contains("code")) {
			if (auto i = p->if_int64())
				code = static_cast<eDiscordError>(*i);
		}
		if (auto p = o->if_contains("message")) {
			if (auto s = p->if_string())
				msg = s->c_str();
		}
		if (auto p = o->if_contains("errors")) {
			if (auto s = p->if_string())
				err = s->c_str();
		}
	}
	switch (code) {
		case eDiscordError::UnknownWebhook:
			throw xUnknownWebhookError(status, msg, err);
		case eDiscordError::InteractionAcknowledged:
			throw xInteractionAcknowledgedError(status, msg, err);
		case eDiscordError::InvalidFormBody:
			throw xInvalidFormBodyError(status, msg, err);
		default:
			throw xDiscordError(status, code, msg, err);
	}
}