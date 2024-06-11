#include "Common.h"
#include "Utils.h"
#include <boost/json.hpp>

namespace json = boost::json;

cSnowflake::cSnowflake(std::uint64_t i) noexcept : m_int(i) {
	auto[ptr, _] = std::to_chars(std::begin(m_str), std::end(m_str), i);
	m_len = ptr - m_str;
}

cSnowflake::cSnowflake(std::string_view s):
	m_int(cUtils::ParseInt<std::uint64_t>(s)),
	m_len(s.copy(m_str, 20)) {}

cSnowflake
tag_invoke(json::value_to_tag<cSnowflake>, const json::value& v) {
	return v.as_string();
}
void
tag_invoke(json::value_from_tag, json::value& v, const cSnowflake& sf) {
	v = sf.ToString();
}
cColor
tag_invoke(json::value_to_tag<cColor>, const json::value& v) {
	return v.to_number<std::int32_t>();
}
void
tag_invoke(json::value_from_tag, json::value& v, cColor c) {
	v = c.ToInt();
}