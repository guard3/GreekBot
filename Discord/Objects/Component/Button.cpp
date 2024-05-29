#include "Button.h"
#include "json.h"

cButton::cButton(const json::value& v) : cButton(v.as_object()) {}
cButton::cButton(const json::object& o):
	m_style(json::value_to<eButtonStyle>(o.at("style"))),
	m_custom_id_or_url(json::value_to<std::string>(o.at(m_style == BUTTON_STYLE_LINK ? "url" : "custom_id"))),
	m_label([&o] {
		auto p = o.if_contains("label");
		return p ? json::value_to<std::string>(*p) : std::string();
	}()) {
	auto p = o.if_contains("disabled");
	m_disabled = p && p->as_bool();
	if ((p = o.if_contains("emoji")))
		m_emoji.emplace(*p);
}

eButtonStyle
tag_invoke(json::value_to_tag<eButtonStyle>, const json::value& v) {
	return (eButtonStyle)v.to_number<std::underlying_type_t<eButtonStyle>>();
}
void
tag_invoke(json::value_from_tag, json::value& v, eButtonStyle style) {
	v = style;
}

cButton
tag_invoke(json::value_to_tag<cButton>, const json::value& v) {
	return cButton{ v };
};