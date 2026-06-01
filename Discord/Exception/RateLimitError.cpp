#include "RateLimitError.h"
#include <boost/json.hpp>
#include <boost/asio/detail/chrono.hpp>

xRateLimitError::xRateLimitError(const std::string& what_arg, const boost::json::object* pObj) : xRateLimitError(what_arg.c_str(), pObj) {}
xRateLimitError::xRateLimitError(const char* what_arg, const boost::json::object* pObj) : std::runtime_error(what_arg) {
	using namespace std::chrono;
	/* If the input JSON value is an object, parse its contents */
	if (const boost::json::value* pValue; pObj) {
		if ((pValue = pObj->if_contains("retry_after")))
			m_retry_after = floor<milliseconds>(duration<double>(pValue->to_number<double>()));
		m_global = (pValue = pObj->if_contains("global")) && pValue->as_bool();
	}
}