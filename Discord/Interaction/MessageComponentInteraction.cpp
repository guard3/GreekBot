#include "MessageComponentInteraction.h"
#include "json.h"

cMessageComponentInteraction::cMessageComponentInteraction(const json::object& obj, const json::object& data) :
	cInteraction(INTERACTION_MESSAGE_COMPONENT, obj),
	m_message(obj.at("message")),
	m_custom_id(json::value_to<std::string>(data.at("custom_id"))),
	m_component_type((eComponentType)data.at("component_type").to_number<int>()) {
	if (auto p = data.if_contains("values"))
		m_values = json::value_to<std::vector<std::string>>(*p);
}
