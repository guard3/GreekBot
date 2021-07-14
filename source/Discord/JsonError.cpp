#include "JsonError.h"

cJsonError::cJsonError(const json::value& v) {
	try {
		m_code = static_cast<eJsonErrorCode>(v.as_object().at("code").as_int64());
		m_message = v.as_object().at("message").as_string().c_str();
	}
	catch (const std::exception&) {
		m_code = DISCORD_JSON_UNKNOWN_ERROR;
		m_message = "Unknown error";
	}
}
