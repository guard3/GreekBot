#include "HttpError.h"
#include <format>
#include <string>
#include <boost/json.hpp>
/* ========== The error category for eHttpStatus ==================================================================== */
class cHttpCategory : public std::error_category {
	static const cHttpCategory ms_instance;
public:
	static const std::error_category& GetInstance() noexcept { return ms_instance; }

	const char* name() const noexcept override { return "Discord HTTP"; }
	std::string message(int ec) const override;
};
const cHttpCategory cHttpCategory::ms_instance;
/* ================================================================================================================== */
std::error_code
make_error_code(eHttpStatus ec) noexcept {
	return { static_cast<int>(ec), cHttpCategory::GetInstance() };
}
/* ========== xHttpError constructors =============================================================================== */
xHttpError::xHttpError(eHttpStatus status) : std::system_error(static_cast<int>(status), cHttpCategory::GetInstance()) {}
xHttpError::xHttpError(eHttpStatus status, const char* what_arg) : std::system_error(static_cast<int>(status), cHttpCategory::GetInstance(), what_arg) {}
xHttpError::xHttpError(eHttpStatus status, const std::string& what_arg) : xHttpError(status, what_arg.c_str()) {}
/* ================================================================================================================== */
std::string
cHttpCategory::message(int ec) const {
	using enum eHttpStatus;
	switch (static_cast<eHttpStatus>(ec)) {
		case Ok:
			return "The request completed successfully";
		case Created:
			return "The entity was created successfully";
		case NoContent:
			return "The request completed successfully but returned no content";
		case NotModified:
			return "The entity was not modified (no action was taken)";
		case BadRequest:
			return "The request was improperly formatted, or the server couldn't understand it";
		case Unauthorized:
			return "The Authorization header was missing or invalid";
		case Forbidden:
			return "The Authorization token you passed did not have permission to the resource";
		case NotFound:
			return "The resource at the location specified doesn't exist";
		case MethodNotAllowed:
			return "The HTTP method used is not valid for the location specified";
		case TooManyRequests:
			return "You are being rate limited";
		case GatewayUnavailable:
			return "There was not a gateway available to process your request";
		default:
			return ec >= 500 && ec <= 599 ? "The server had an error processing your request" : std::format("Unexpected status code {}", ec);
	}
}