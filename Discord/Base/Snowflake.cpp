#include "Snowflake.h"
#include <format>
#include <boost/json.hpp>
/* ================================================================================================================== */
namespace json = boost::json;
/* ========== Implement the bad snowflake error constructor here to avoid including <format> everywhere ============= */
xBadSnowflakeError::xBadSnowflakeError(std::string_view arg) : std::invalid_argument(std::format("{:?} is not a valid snowflake", arg)) {}
/* ========== JSON conversion ======================================================================================= */
cSnowflake
tag_invoke(json::value_to_tag<cSnowflake>, const json::value& v) {
	return v.as_string();
}
void
tag_invoke(json::value_from_tag, json::value& v, const cSnowflake& sf) {
	v = sf.ToString();
}