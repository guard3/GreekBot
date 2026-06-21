#include "ActionRow.h"
#include <boost/json.hpp>

namespace json = boost::json;

cActionRow::cActionRow(const json::value& v) : cActionRow(v.as_object()) {}
cActionRow::cActionRow(const json::object& o) : cComponentBase(o), m_components(json::value_to<std::vector<child_component_type>>(o.at("components"))) {}

/** @name JSON value to object conversion
 */
/// @{
cActionRow
tag_invoke(json::value_to_tag<cActionRow>, const json::value& v) {
	return cActionRow{ v };
}
/// @}

/** @name JSON value from object conversion
 */
/// @{
void
tag_invoke(json::value_from_tag, json::value& v, const cActionRow& row) {
	json::object& obj = v.emplace_object();
	obj.reserve(3);
	obj.emplace("type", COMPONENT_ACTION_ROW);
	obj.emplace("id", row.GetId());
	json::value_from(row.GetComponents(), obj["components"]);
}
/// @}
