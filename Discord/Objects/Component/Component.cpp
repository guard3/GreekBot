#include "Component.h"
#include "json.h"

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
		{ "custom_id",  ti.m_custom_id       },
		{ "label",      ti.m_label           },
		{ "style",      ti.m_style           },
		{ "min_length", ti.m_min_length      },
		{ "max_length", ti.m_max_length      }
	};
}
void
tag_invoke(const json::value_from_tag&, json::value& v, const cActionRow& row) {
	json::object& obj = v.emplace_object();
	obj.emplace("type", COMPONENT_ACTION_ROW);
	json::value_from(row.GetComponents(), obj["components"]);
}