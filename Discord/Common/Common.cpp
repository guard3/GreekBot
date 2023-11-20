#include "Common.h"
#include "Utils.h"
#include "json.h"

cSnowflake::cSnowflake(uint64_t i) noexcept:
	m_int(i) {
	auto result = std::to_chars(std::begin(m_str), std::end(m_str), i);
	m_len = result.ptr - m_str;
}

cSnowflake::cSnowflake(std::string_view s):
	m_int(cUtils::ParseInt<uint64_t>(s)),
	m_len(s.copy(m_str, 20)) {}

cSnowflake
tag_invoke(json::value_to_tag<cSnowflake>, const json::value& v) {
	return json::value_to<std::string_view>(v);
}
cColor
tag_invoke(json::value_to_tag<cColor>, const json::value& v) {
	return v.to_number<int32_t>();
}