#include "TextInput.h"
#include "json.h"
/* ================================================================================================================== */
eTextInputStyle
tag_invoke(json::value_to_tag<eTextInputStyle>, const json::value& v) {
	return (eTextInputStyle)v.to_number<std::underlying_type_t<eTextInputStyle>>();
}
void
tag_invoke(json::value_from_tag, json::value& v, eTextInputStyle style) {
	v = style;
}
/* ================================================================================================================== */
cTextInput::cTextInput(const json::value& v): cTextInput(v.as_object()) {}
cTextInput::cTextInput(const json::object& o):
	m_custom_id(json::value_to<std::string>(o.at("custom_id"))),
	m_label([&o] {
		auto p = o.if_contains("label");
		return p ? json::value_to<std::string>(*p) : std::string();
	}()),
	m_value(json::value_to<std::string>(o.at("value"))),
	m_style([&o] {
		auto p = o.if_contains("style");
		return p ? json::value_to<eTextInputStyle>(*p) : TEXT_INPUT_SHORT;
	}()),
	m_required([&o] {
		auto p = o.if_contains("required");
		return p && p->as_bool();
	}()),
	m_min_length([&o] {
		auto p = o.if_contains("min_length");
		return p ? p->to_number<std::uint16_t>() : 0;
	}()),
	m_max_length([&o] {
		auto p = o.if_contains("max_length");
		return p ? p->to_number<std::uint16_t>() : 4000;
	}()) {}
/* ================================================================================================================== */
cTextInput
tag_invoke(json::value_to_tag<cTextInput>, const json::value& v) {
	return cTextInput{ v };
}