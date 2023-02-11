#include "Component.h"
#include "json.h"

cComponent::cComponent(const json::object& o) : m_type((eComponentType)o.at("type").as_int64()) {}
cComponent::cComponent(const json::value& v) : cComponent(v.as_object()) {}

json::object
cComponent::ToJson() const { return {{ "type", (int)m_type }}; }

json::object
cBaseButton::ToJson() const {
	json::object obj {
		{ "type", (int)COMPONENT_BUTTON },
		{ "style", (int)m_style },
		{ "label", m_label },
		{ "disabled", m_disabled },
		{ m_style == BUTTON_STYLE_LINK ? "url" : "custom_id", m_value }
	};
	if (m_emoji)
		obj["emoji"] = m_emoji->ToJson();
	return obj;
}

json::object
cSelectOption::ToJson() const {
	json::object obj {
		{ "label", m_label },
		{ "value", m_value }
	};
	if (!m_description.empty())
		obj["description"] = m_description;
	if (m_emoji)
		obj["emoji"] = m_emoji->ToJson();
	return obj;
}

json::object
cSelectMenu::ToJson() const {
	json::object obj = cComponent::ToJson();
	if (!custom_id.empty())
		obj["custom_id"] = custom_id;
	if (!placeholder.empty())
		obj["placeholder"] = placeholder;
	if (!options.empty()) {
		json::array opts;
		opts.reserve(options.size());
		for (auto& o : options)
			opts.push_back(o.ToJson());
		obj["options"] = std::move(opts);
	}
	return obj;
}

json::object cActionRow::ToJson() const {
	json::object obj = cComponent::ToJson();
	if (!Components.empty()) {
		json::array components;
		components.reserve(Components.size());
		for (chComponent c : Components)
			components.push_back(c->ToJson());
		obj["components"] = std::move(components);
	}
	return obj;
}
