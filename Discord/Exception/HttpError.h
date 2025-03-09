#ifndef DISCORD_HTTPERROR_H
#define DISCORD_HTTPERROR_H
#include <chrono>
#include <system_error>
/* ========== Boost forward declarations ============================================================================ */
namespace boost::json {
	class object;
}

/* ========== The HTTP status codes mentioned in the Discord documentation ========================================== */
enum class eHttpStatus {
	Ok                 = 200, // The request completed successfully.
	Created            = 201, // The entity was created successfully.
	NoContent          = 204, // The request completed successfully but returned no content.
	NotModified        = 304, // The entity was not modified (no action was taken).
	BadRequest         = 400, // The request was improperly formatted, or the server couldn't understand it.
	Unauthorized       = 401, // The Authorization header was missing or invalid.
	Forbidden          = 403, // The Authorization token you passed did not have permission to the resource.
	NotFound           = 404, // The resource at the location specified doesn't exist.
	MethodNotAllowed   = 405, // The HTTP method used is not valid for the location specified.
	TooManyRequests    = 429, // You are being rate limited, see Rate Limits.
	GatewayUnavailable = 502, // There was not a gateway available to process your request. Wait a bit and retry.
	                  // 5XX  // The server had an error processing your request (these are rare).
};
/* ========== Make eHttpStatus implicitly convertible to std::error_code ============================================ */
template<>
struct std::is_error_code_enum<eHttpStatus> : std::true_type {};

std::error_code make_error_code(eHttpStatus) noexcept;

/* ========== A specialized exception type for HTTP status codes ==================================================== */
struct xHttpError : std::system_error {
	xHttpError(eHttpStatus);
	xHttpError(eHttpStatus, const char* what_arg);
	xHttpError(eHttpStatus, const std::string& what_arg);
};
/* ========== A specialized exception type for 'Too many requests' status code ====================================== */
class xRateLimitError : public xHttpError {
	std::chrono::milliseconds m_retry_after;
	bool                      m_global{};

public:
	xRateLimitError(const char* what_arg, const boost::json::object* obj = nullptr);
	xRateLimitError(const std::string& what_arg, const boost::json::object* obj = nullptr);

	auto retry_after() const noexcept { return m_retry_after; }
	auto      global() const noexcept { return m_global;      }
};
#endif /* DISCORD_HTTPERROR_H */