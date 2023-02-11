#include "Component.h"
#include "json.h"

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
	json::object obj {
		{ "type", (int)COMPONENT_SELECT_MENU }
	};
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

json::object
cActionRow::ToJson() const {
	json::object obj {
		{ "type", (int)COMPONENT_ACTION_ROW }
	};
	if (!m_components.empty()) {
		json::array a;
		a.reserve(m_components.size());
		for (auto& c : m_components)
			a.emplace_back(c.ToJson());
		obj["components"] = std::move(a);
	}
	return obj;
}

template<>
cComponent::cComponent(cButton<BUTTON_STYLE_LINK> b):
	m_component(std::move(b)),
	m_func([](const void* p) -> json::object {
		return reinterpret_cast<const cButton<BUTTON_STYLE_LINK>*>(p)->ToJson();
	}) {}
template<>
cComponent::cComponent(cButton<BUTTON_STYLE_SUCCESS> b):
	m_component(std::move(b)),
	m_func([](const void* p) -> json::object {
		return reinterpret_cast<const cButton<BUTTON_STYLE_SUCCESS>*>(p)->ToJson();
	}) {}
template<>
cComponent::cComponent(cButton<BUTTON_STYLE_SECONDARY> b):
	m_component(std::move(b)),
	m_func([](const void* p) -> json::object {
		return reinterpret_cast<const cButton<BUTTON_STYLE_SECONDARY>*>(p)->ToJson();
	}) {}
template<>
cComponent::cComponent(cButton<BUTTON_STYLE_PRIMARY> b):
	m_component(std::move(b)),
	m_func([](const void* p) -> json::object {
		return reinterpret_cast<const cButton<BUTTON_STYLE_PRIMARY>*>(p)->ToJson();
	}) {}
template<>
cComponent::cComponent(cButton<BUTTON_STYLE_DANGER> b):
	m_component(std::move(b)),
	m_func([](const void* p) -> json::object {
		return reinterpret_cast<const cButton<BUTTON_STYLE_DANGER>*>(p)->ToJson();
	}) {}
cComponent::cComponent(cSelectMenu m):
	m_component(std::move(m)),
	m_func([](const void* p) -> json::object {
		return reinterpret_cast<const cSelectMenu*>(p)->ToJson();
	}){}

json::object
cComponent::ToJson() const  {
	const void* value;
	switch (m_component.index()) {
		case 0:  value = &std::get<0>(m_component); break;
		case 1:  value = &std::get<1>(m_component); break;
		case 2:  value = &std::get<2>(m_component); break;
		case 3:  value = &std::get<3>(m_component); break;
		case 4:  value = &std::get<4>(m_component); break;
		default: value = &std::get<5>(m_component); break;
	}
	return m_func(value);
}
