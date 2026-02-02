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
cLabelChildComponent
tag_invoke(json::value_to_tag<cLabelChildComponent>, const json::value& v) {
	switch (auto& obj = v.as_object(); obj.at("type").to_number<int>()) {
	case COMPONENT_TEXT_INPUT:
		return cLabelChildComponent(std::in_place_type<cTextInput>, obj);
	default:
		return cLabelChildComponent(std::in_place_type<cUnsupportedComponent>, v);
	}
}

cPartialLabel
tag_invoke(json::value_to_tag<cPartialLabel>, const json::value& v) {
	return cPartialLabel{ v };
}
/// @}

/** @name JSON value from object conversion
 */
/// @{
void
tag_invoke(json::value_from_tag, json::value& v, const cLabelChildComponent& comp) {
	std::visit([&v](const auto& comp) { json::value_from(comp, v); }, comp);
}

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
/// @}