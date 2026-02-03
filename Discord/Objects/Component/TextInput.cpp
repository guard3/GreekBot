#include "ComponentType.h"
#include "TextInput.h"
#include <boost/json.hpp>

namespace json = boost::json;

cPartialTextInput::cPartialTextInput(const json::value& v): cPartialTextInput(v.as_object()) {}
cPartialTextInput::cPartialTextInput(const json::object& o):
	cComponentBase(o),
	m_custom_id(json::value_to<std::string>(o.at("custom_id"))),
	m_value(json::value_to<std::string>(o.at("value"))) {}

/** @name JSON value to object conversion
 */
/// @{
eTextInputStyle
tag_invoke(json::value_to_tag<eTextInputStyle>, const json::value& v) {
	return static_cast<eTextInputStyle>(v.to_number<std::underlying_type_t<eTextInputStyle>>());
}

cPartialTextInput
tag_invoke(json::value_to_tag<cPartialTextInput>, const json::value& v) {
	return cPartialTextInput{ v };
}
/// @}

/** @name JSON value from object conversion
 */
/// @{
void
tag_invoke(json::value_from_tag, json::value& v, eTextInputStyle style) {
	v = std::to_underlying(style);
}

void
tag_invoke(json::value_from_tag, json::value& v, const cTextInput& ti) {
	auto& obj = v.emplace_object();
	obj.reserve(7);
	obj.emplace("type", COMPONENT_TEXT_INPUT);
	obj.emplace("id", ti.GetId());
	obj.emplace("custom_id", ti.GetCustomId());
	obj.emplace("style", ti.GetStyle());
	/* Fill out optional values */
	auto min_length = std::min<std::uint16_t>(ti.GetMinLength(), 4000);
	if (ti.IsRequired()) {
		/* If the text input is required, then its minimum length should be 1
		 * The only reason we do that is for a more factually correct error message in the discord client */
		min_length = std::max<std::uint16_t>(min_length, 1);
	} else {
		obj.emplace("required", false);
	}
	if (min_length != 0)
		obj.emplace("min_length", min_length);
	/* Constrain max length */
	if (auto max_length = std::max<std::uint16_t>(ti.GetMaxLength(), 1); max_length < 4000)
		obj.emplace("max_length", std::max(min_length, max_length));
}
/// @}
