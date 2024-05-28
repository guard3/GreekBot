#include "Component.h"
#include "json.h"
/* ================================================================================================================== */
cActionRow::cActionRow(const json::value& v) : cActionRow(v.as_object()) {}
cActionRow::cActionRow(const json::object& o) : m_components(json::value_to<std::vector<cComponent>>(o.at("components"))) {}
/* ================================================================================================================== */
cComponent
tag_invoke(json::value_to_tag<cComponent>, const json::value& v) {
	switch (v.at("type").to_number<int>()) {
		case COMPONENT_BUTTON:
			if (v.at("style").to_number<int>() == 5)
				return cComponent(std::in_place_type<cLinkButton>, v);
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
		{ "type",     COMPONENT_BUTTON },
		{ "style",    b.GetStyle()     },
		{ "label",    b.GetLabel()     },
		{ "disabled", b.IsDisabled()   },
		{ b.GetStyle() == 5 ? "url" : "custom_id", b.GetCustomId() }
	};
	if (auto e = b.GetEmoji(); e)
		json::value_from(*e, obj["emoji"]);
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
	v = {
		{ "type",       COMPONENT_TEXT_INPUT },
		{ "custom_id",  ti.GetCustomId()     },
		{ "label",      ti.GetLabel()        },
		{ "style",      ti.GetStyle()        },
		{ "min_length", ti.GetMinLength()    },
		{ "max_length", ti.GetMaxLength()    },
		{ "required",   ti.IsRequired()      }
	};
}