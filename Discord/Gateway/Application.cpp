#include "Application.h"
#include "json.h"

eApplicationFlag
tag_invoke(json::value_to_tag<eApplicationFlag>, const json::value& v) {
	return static_cast<eApplicationFlag>(v.to_number<int>());
}

cApplication::cApplication(const json::value& v) : m_id(json::value_to<cSnowflake>(v.at("id"))), m_flags(json::value_to<eApplicationFlag>(v.at("flags"))) {}

cApplication
tag_invoke(json::value_to_tag<cApplication>, const json::value& v) {
	return v;
}