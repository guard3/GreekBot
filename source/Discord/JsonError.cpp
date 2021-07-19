#include "JsonError.h"

cJsonError::cJsonError(const json::object& obj) {
	const json::string& msg = obj.at("message").as_string();
	m_code = static_cast<eJsonErrorCode>(obj.at("code").as_int64());
	if ((m_message = reinterpret_cast<char*>(malloc(msg.size() + 1))))
		strcpy(m_message, msg.c_str());
}

cJsonError::cJsonError(const cJsonError& e) {
	m_code = e.m_code;
	m_message = nullptr;
	if (e.m_message) {
		if ((m_message = reinterpret_cast<char*>(malloc(strlen(e.m_message) + 1))))
			strcpy(m_message, e.m_message);
	}
}

cJsonError::cJsonError(cJsonError&& e) {
	m_message = e.m_message;
	m_code = e.m_code;
	e.m_message = nullptr;	
}

cJsonError::~cJsonError() {
	free(m_message);
}

cJsonError& cJsonError::operator=(cJsonError e) {
	m_code = e.m_code;
	auto tmp = m_message;
	m_message = e.m_message;
	e.m_message = tmp;
	return *this;
}
