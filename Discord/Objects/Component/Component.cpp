#include "Component.h"
#include "ComponentType.h"
#include <boost/json.hpp>
/* ================================================================================================================== */
namespace json = boost::json;
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
