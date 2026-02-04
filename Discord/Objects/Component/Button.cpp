#include "Button.h"
#include "ComponentType.h"
#include <boost/json.hpp>

namespace json = boost::json;

cButton::cButton(const json::value& v) : cButton(v.as_object()) {}
cButton::cButton(const json::object& o):
	cComponentBase(o),
	m_style(json::value_to<eButtonStyle>(o.at("style"))),
	m_disabled([&o] {
		auto p = o.if_contains("disabled");
		return p && p->as_bool();
	}()),
	m_custom_id_or_url(json::value_to<std::string>(o.at(m_style == eButtonStyle::Link ? "url" : "custom_id"))),
	m_label([&o] {
		auto p = o.if_contains("label");
		return p ? json::value_to<std::string>(*p) : std::string();
	}()),
	m_emoji([&o] {
		auto p = o.if_contains("emoji");
		return p ? std::optional<cEmoji>(std::in_place, *p) : std::nullopt;
	}()) {}

/** @name JSON value to object conversion
 */
/// @{
eButtonStyle
tag_invoke(json::value_to_tag<eButtonStyle>, const json::value& v) {
	return static_cast<eButtonStyle>(v.to_number<std::underlying_type_t<eButtonStyle>>());
}

cButton
tag_invoke(json::value_to_tag<cButton>, const json::value& v) {
	return cButton{ v };
};
/// @}

/** @name JSON value from object conversion
 */
/// @{
void
tag_invoke(json::value_from_tag, json::value& v, eButtonStyle style) {
	v = std::to_underlying(style);
}

void
tag_invoke(json::value_from_tag, json::value& v, const cButton& button) {
	json::object& obj = v.emplace_object();
	obj.reserve(7);

	obj.emplace("type", COMPONENT_BUTTON);
	obj.emplace("id", button.GetId());
	obj.emplace("disabled", button.IsDisabled());
	json::value_from(button.GetStyle(), obj["style"]);
	
	if (auto label = button.GetLabel(); !label.empty())
		obj.emplace("label", label);

	if (button.GetStyle() == eButtonStyle::Link)
		obj.emplace("url", button.GetUrl());
	else
		obj.emplace("custom_id", button.GetCustomId());

	if (auto pEmoji = button.GetEmoji().Get())
		json::value_from(*pEmoji, obj["emoji"]);
}
/// @}