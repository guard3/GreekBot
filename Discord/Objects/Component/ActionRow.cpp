#include "ActionRow.h"
#include "ComponentType.h"
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

cActionRow::child_component_type
tag_invoke(json::value_to_tag<cActionRow::child_component_type>, const json::value& v) {
	switch (v.at("type").to_number<int>()) {
	case COMPONENT_BUTTON:
		return cActionRow::child_component_type(std::in_place_type<cButton>, v);
	case COMPONENT_SELECT_MENU:
		return cActionRow::child_component_type(std::in_place_type<cSelectMenu>, v);
	default:
		return cActionRow::child_component_type(std::in_place_type<cUnsupportedComponent>, v);
	}
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

void
tag_invoke(json::value_from_tag, json::value& v, const cActionRow::child_component_type& comp) {
	std::visit([&v](auto&& c) { json::value_from(c, v); }, comp);
}
/// @}