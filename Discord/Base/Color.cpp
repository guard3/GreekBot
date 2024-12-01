#include "Color.h"
#include <boost/json.hpp>
/* ================================================================================================================== */
namespace json = boost::json;
/* ================================================================================================================== */
cColor
tag_invoke(json::value_to_tag<cColor>, const json::value& v) {
	return v.to_number<std::int32_t>();
}
void
tag_invoke(json::value_from_tag, json::value& v, cColor c) {
	v = c.ToInt();
}