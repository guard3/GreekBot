#include "Label.h"
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
/// @}

/** @name JSON value from object conversion
 */
/// @{
void
tag_invoke(json::value_from_tag, json::value& v, const cLabel& label) {
	auto& obj = v.emplace_object();
	obj.reserve(5);

	obj.emplace("type", COMPONENT_LABEL);
	obj.emplace("id", label.GetId());
	obj.emplace("label", label.GetLabel());
	json::value_from(label.GetComponent(), obj["component"]);

	if (auto desc = label.GetDescription(); !desc.empty())
		obj.emplace("description", desc);
}
/// @}
