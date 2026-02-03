#include "Component.h"
#include "ComponentType.h"
#include <boost/json.hpp>
/* ================================================================================================================== */
namespace json = boost::json;
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

cContentComponent
tag_invoke(json::value_to_tag<cContentComponent>, const json::value& v) {
	switch (v.at("type").to_number<int>()) {
	case COMPONENT_TEXT_DISPLAY:
		return cContentComponent(std::in_place_type<cTextDisplay>, v);
	default:
		return cContentComponent(std::in_place_type<cUnsupportedComponent>, v);
	}
}

void
tag_invoke(json::value_from_tag, json::value& v, const cContentComponent& cmp) {
	std::visit([&v](const auto& cmp) { json::value_from(cmp, v); }, cmp);
}

cLayoutPartialComponent
tag_invoke(json::value_to_tag<cLayoutPartialComponent>, const json::value& v) {
	switch (v.at("type").to_number<int>()) {
	case COMPONENT_ACTION_ROW:
		return cLayoutPartialComponent(std::in_place_type<cActionRow>, v);
	case COMPONENT_LABEL:
		return cLayoutPartialComponent(std::in_place_type<cPartialLabel>, v);
	default:
		return cLayoutPartialComponent(std::in_place_type<cUnsupportedComponent>, v);
	}
}

void
tag_invoke(json::value_from_tag, json::value& v, const cLayoutComponent& cmp) {
	std::visit([&v](const auto& cmp) { json::value_from(cmp, v); }, cmp);
}
