#include "Component.h"
#include <boost/json.hpp>
/* ================================================================================================================== */
namespace json = boost::json;
/* ================================================================================================================== */
eComponentType
tag_invoke(json::value_to_tag<eComponentType>, const json::value& v) {
	return (eComponentType)v.to_number<std::underlying_type_t<eComponentType>>();
}
void
tag_invoke(json::value_from_tag, json::value& v, eComponentType t) {
	v = t;
}
/* ================================================================================================================== */
cActionRow::cActionRow(const json::value& v) : cActionRow(v.as_object()) {}
cActionRow::cActionRow(const json::object& o) : m_components(json::value_to<std::vector<cComponent>>(o.at("components"))) {}
/* ================================================================================================================== */
cComponent
tag_invoke(json::value_to_tag<cComponent>, const json::value& v) {
	switch (v.at("type").to_number<int>()) {
		case COMPONENT_BUTTON:
			return cComponent(std::in_place_type<cButton>, v);
		case COMPONENT_SELECT_MENU:
			return cComponent(std::in_place_type<cSelectMenu>, v);
		case COMPONENT_TEXT_INPUT:
			return cComponent(std::in_place_type<cTextInput>, v);
		default:
			return cComponent(std::in_place_type<cUnsupportedComponent>, v);
	}
}
void
tag_invoke(json::value_from_tag, json::value& v, const cComponent& comp) {
	std::visit([&v](auto&& c) { json::value_from(c, v); }, comp);
}
/* ================================================================================================================== */
cActionRow
tag_invoke(json::value_to_tag<cActionRow>, const json::value& v) {
	return cActionRow{ v };
}
void
tag_invoke(json::value_from_tag, json::value& v, const cActionRow& row) {
	json::object& obj = v.emplace_object();
	obj.emplace("type", COMPONENT_ACTION_ROW);
	json::value_from(row.GetComponents(), obj["components"]);
}
/* ================================================================================================================== */
void
tag_invoke(json::value_from_tag, json::value& v, const cButton& b) {
	json::object& obj = v.emplace_object();
	obj = {
		{ "type",  COMPONENT_BUTTON },
		{ "style", b.GetStyle()     }
	};
	if (auto label = b.GetLabel(); !label.empty())
		obj.emplace("label", label);
	if (b.IsDisabled())
		obj.emplace("disabled", true);
	if (b.GetStyle() == BUTTON_STYLE_LINK)
		obj.emplace("url", b.GetUrl());
	else
		obj.emplace("custom_id", b.GetCustomId());
	if (auto pEmoji = b.GetEmoji().Get())
		json::value_from(*pEmoji, obj["emoji"]);
}
void
tag_invoke(json::value_from_tag, json::value& v, const cSelectMenu& sm) {
	json::object& obj = v.emplace_object();
	obj = {
		{ "type",      COMPONENT_SELECT_MENU },
		{ "custom_id", sm.GetCustomId()      },
	};
	json::value_from(sm.GetOptions(), obj["options"]);
	if (auto s = sm.GetPlaceholder(); !s.empty())
		obj.emplace("placeholder", s);
}
void
tag_invoke(json::value_from_tag, json::value& v, const cTextInput& ti) {
	auto& obj = v.emplace_object();
	obj.reserve(7);
	obj.emplace("type", COMPONENT_TEXT_INPUT);
	obj.emplace("custom_id", ti.GetCustomId());
	obj.emplace("label", ti.GetLabel());
	obj.emplace("style", ti.GetStyle());
	/* Fill out optional values */
	auto min_length = std::min(ti.GetMinLength(), std::uint16_t(4000));
	if (ti.IsRequired()) {
		/* If the text input is required, then its minimum length should be 1
		 * The only reason we do that is for a more factually correct error message in the discord client */
		min_length = std::max(min_length, std::uint16_t(1));
	} else {
		obj.emplace("required", false);
	}
	if (min_length != 0)
		obj.emplace("min_length", min_length);
	/* Constrain max length */
	auto max_length = std::max(ti.GetMaxLength(), std::uint16_t(1));
	if (max_length < 4000)
		obj.emplace("max_length", std::max(min_length, max_length));
}