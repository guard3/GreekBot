#include "Component.h"
#include "json.h"

cTextInput::cTextInput(const json::value& v) : cTextInput(v.as_object()) {}
cTextInput::cTextInput(const json::object& o) :
	m_custom_id(json::value_to<std::string>(o.at("custom_id"))),
	m_value(json::value_to<std::string>(o.at("value"))) {}

cActionRow::cActionRow(const json::value& v) : cActionRow(v.as_object()) {}
cActionRow::cActionRow(const json::object& o) : m_components(json::value_to<std::vector<cComponent>>(o.at("components"))) {}

json::result_for<cButton, json::value>::type
tag_invoke(json::try_value_to_tag<cButton>, const json::value& v) {
	return json::error::exception;
}
json::result_for<cLinkButton, json::value>::type
tag_invoke(json::try_value_to_tag<cLinkButton>, const json::value&) {
	return json::error::exception;
}
json::result_for<cSelectMenu, json::value>::type
tag_invoke(json::try_value_to_tag<cSelectMenu>, const json::value&) {
	return json::error::exception;
}
cTextInput
tag_invoke(json::value_to_tag<cTextInput>, const json::value& v) {
	return cTextInput{ v };
}

void
tag_invoke(const json::value_from_tag&, json::value& v, const cButton& b) {
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
tag_invoke(const json::value_from_tag&, json::value& v, const cSelectOption& so) {
	json::object& obj = v.emplace_object();
	obj = {
		{ "label", so.m_label },
		{ "value", so.m_value }
	};
	if (!so.m_description.empty())
		obj.emplace("description", so.m_description);
	if (so.m_emoji)
		json::value_from(*so.m_emoji, obj["emoji"]);
}
void
tag_invoke(const json::value_from_tag&, json::value& v, const cSelectMenu& sm) {
	json::object& obj = v.emplace_object();
	obj = {
		{ "type",      COMPONENT_SELECT_MENU },
		{ "custom_id", sm.m_custom_id        },
	};
	json::value_from(sm.m_options, obj["options"]);
	if (!sm.m_placeholder.empty())
		obj.emplace("placeholder", sm.m_placeholder);
}
void
tag_invoke(const json::value_from_tag&, json::value& v, const cTextInput& ti) {
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
void
tag_invoke(const json::value_from_tag&, json::value& v, const cActionRow& row) {
	json::object& obj = v.emplace_object();
	obj.emplace("type", COMPONENT_ACTION_ROW);
	json::value_from(row.GetComponents(), obj["components"]);
}
cActionRow
tag_invoke(json::value_to_tag<cActionRow>, const json::value& v) {
	return cActionRow{ v };
}