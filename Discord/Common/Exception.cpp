#include "Exception.h"
#include "json.h"
using namespace std::chrono;

xDiscordError::xDiscordError(const json::value& v) : xDiscordError(v.as_object()) {}
xDiscordError::xDiscordError(const json::object& o):
	xSystemError(o.at("message").as_string().c_str(), o.at("code").to_number<int>()),
	m_errors([](const json::object& o) -> std::string {
		try {
			return json::serialize(o.at("errors"));
		}
		catch (...) {
			return "{}";
		}
	}(o)) {}

xRateLimitError::xRateLimitError(const json::value& v) : xRateLimitError(v.as_object()) {}
xRateLimitError::xRateLimitError(const json::object& o):
	std::runtime_error(o.at("message").as_string().c_str()),
	m_retry_after(duration_cast<milliseconds>(duration<double>(o.at("retry_after").as_double()))),
	m_global(o.at("global").as_bool()) {}