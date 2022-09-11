#include "Discord.h"
#include "beast.h"
#include "json.h"
#include <tuple>

xSystemError::xSystemError(const json::object& o) : std::runtime_error([](const json::value* v) -> const char* {
	if (const json::string* s; v && (s = v->if_string()))
		return s->c_str();
	return "An error occurred";
}(o.if_contains("message"))), m_code(0) {
	if (auto p = o.if_contains("code"))
		if (auto i = p->if_int64())
			m_code = *i;
}

xSystemError::xSystemError(const json::value& v) : xSystemError(v.as_object()) {}

xSystemError::xSystemError(const boost::system::system_error& e) : xSystemError(e.what(), e.code().value()) {}

xDiscordError::xDiscordError(const json::object& o) : xSystemError(o) {
	try {
		m_errors = json::serialize(o.at("errors"));
	}
	catch (...) {}
}

xDiscordError::xDiscordError(const json::value& v) : xDiscordError(v.as_object()) {}