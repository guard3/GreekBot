#include "Common.h"
#include "json.h"
#include <fmt/format.h>

cSnowflake
tag_invoke(json::value_to_tag<cSnowflake>, const json::value& v) {
	return json::value_to<std::string_view>(v);
}
cColor
tag_invoke(json::value_to_tag<cColor>, const json::value& v) {
	return v.to_number<int32_t>();
}

fmt::format_context::iterator fmt::formatter<cSnowflake>::format(const cSnowflake& snowflake, format_context& ctx) const {
	return formatter<string_view>::format(snowflake.ToString(), ctx);
}