#include "Label.h"
#include "Component.h"
#include "ComponentType.h"
#include <boost/json.hpp>

namespace json = boost::json;

cPartialLabel::cPartialLabel(const json::value& v) : cPartialLabel(v.as_object()) {}
cPartialLabel::cPartialLabel(const json::object& o):
	cComponentBase(o),
	m_component(json::value_to<child_component_type>(o.at("component"))) {}

/** @name JSON value to object conversion
 */
/// @{
cPartialLabel
tag_invoke(json::value_to_tag<cPartialLabel>, const json::value& v) {
	return cPartialLabel{ v };
}

cPartialLabel::child_component_type
tag_invoke(json::value_to_tag<cPartialLabel::child_component_type>, const json::value& v) {
	switch (auto& obj = v.as_object(); obj.at("type").to_number<int>()) {
	case COMPONENT_TEXT_INPUT:
		return cPartialLabel::child_component_type(std::in_place_type<cPartialTextInput>, obj);
	default:
		return cPartialLabel::child_component_type(std::in_place_type<cUnsupportedComponent>, v);
	}
}
/// @}

/** @name JSON value from object conversion
 */
/// @{
void
tag_invoke(json::value_from_tag, json::value& v, const cLabel& label) {
	auto& obj = v.emplace_object();
	obj.reserve(5);

	obj.emplace("type", COMPONENT_LABEL);
	obj.emplace("label", label.GetLabel());
	json::value_from(label.GetComponent(), obj["component"]);

	if (auto id = label.GetId(); id != 0)
		obj.emplace("id", id);
	if (auto desc = label.GetDescription(); !desc.empty())
		obj.emplace("description", desc);
}

void
tag_invoke(json::value_from_tag, json::value& v, const cLabel::child_component_type& comp) {
	std::visit([&v](const auto& comp) { json::value_from(comp, v); }, comp);
}
/// @}