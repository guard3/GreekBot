#include "Button.h"
#include "json.h"

cButton::cButton(const json::value& v) : cButton(v.as_object()) {}
cButton::cButton(const json::object& o):
	m_style(o.at("style").to_number<int>()),
	m_value(json::value_to<std::string>(o.at(m_style == 5 ? "url" : "custom_id"))),
	m_label([&o] {
		auto p = o.if_contains("label");
		return p ? json::value_to<std::string>(*p) : std::string();
	}()) {
	auto p = o.if_contains("disabled");
	m_disabled = p && p->as_bool();
	if ((p = o.if_contains("emoji")))
		m_emoji.emplace(*p);
}

cLinkButton::cLinkButton(const json::value& v) : cButton(v) {}
cLinkButton::cLinkButton(const json::object& o) : cButton(o) {}

cButton
tag_invoke(json::value_to_tag<cButton>, const json::value& v) {
	return cButton{ v };
};
cLinkButton
tag_invoke(json::value_to_tag<cLinkButton>, const json::value& v) {
	return cLinkButton{ v };
};