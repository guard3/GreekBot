#ifndef GREEKBOT_EXCEPTION_H
#define GREEKBOT_EXCEPTION_H
#include <chrono>
#include <memory>
#include <stdexcept>

namespace boost::json {
	class object;
	class value;
}

namespace detail {
	[[noreturn]]
	void throw_rate_limit_exception(unsigned, const boost::json::value&);
	[[noreturn]]
	void throw_discord_exception(unsigned, const boost::json::value& o);
}

class xHTTPError : public std::runtime_error {
private:
	unsigned m_status;
public:
	explicit xHTTPError(unsigned status, const char* msg = nullptr) noexcept : std::runtime_error(msg ? msg : "HTTP error"), m_status(status) {}

	unsigned status() const noexcept { return m_status; }
};

class xRateLimitError final : public xHTTPError {
private:
	std::chrono::milliseconds m_retry_after;
	bool                      m_global;

	explicit xRateLimitError(unsigned, const char*, std::chrono::milliseconds, bool);
public:
	xRateLimitError(const xRateLimitError&) noexcept = default;
	xRateLimitError(xRateLimitError&&) noexcept = default;

	std::chrono::milliseconds retry_after() const noexcept;
	bool                           global() const noexcept;

	friend void detail::throw_rate_limit_exception(unsigned, const boost::json::value&);
};

/* I'm going to be filling these as I need them */
enum class eDiscordError : std::uint32_t {
	GeneralError            = 0,
	UnknownWebhook          = 10015,
	InteractionAcknowledged = 40060,
	InvalidFormBody         = 50035
};

class xDiscordError : public xHTTPError {
protected:
	eDiscordError           m_code;
	std::shared_ptr<char[]> m_errors;

	xDiscordError(unsigned, eDiscordError, const char*, const char*);
public:
	xDiscordError(const xDiscordError&) noexcept = default;
	xDiscordError(xDiscordError&&) noexcept = default;

	const char* errors() const noexcept;
	eDiscordError code() const noexcept;

	friend void detail::throw_discord_exception(unsigned, const boost::json::value&);
};

class xUnknownWebhookError final : public xDiscordError {
private:
	xUnknownWebhookError(unsigned, const char*, const char*);
public:
	xUnknownWebhookError(const xUnknownWebhookError&) noexcept = default;
	xUnknownWebhookError(xUnknownWebhookError&&) noexcept = default;
	friend void detail::throw_discord_exception(unsigned, const boost::json::value&);
};
typedef xUnknownWebhookError xUnknownInteractionError;

class xInteractionAcknowledgedError final : public xDiscordError {
private:
	xInteractionAcknowledgedError(unsigned, const char*, const char*);
public:
	xInteractionAcknowledgedError();
	friend void detail::throw_discord_exception(unsigned, const boost::json::value&);
};
class xInvalidFormBodyError final : public xDiscordError {
private:
	xInvalidFormBodyError(unsigned, const char*, const char*);
public:
	xInvalidFormBodyError();
	friend void detail::throw_discord_exception(unsigned, const boost::json::value&);
};
#endif //GREEKBOT_EXCEPTION_H