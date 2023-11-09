#include "ModalSubmitInteraction.h"
#include "json.h"

cModalSubmitInteraction::cModalSubmitInteraction(const json::value& v) : cModalSubmitInteraction(v.as_object()) {}
cModalSubmitInteraction::cModalSubmitInteraction(const json::object& o) : cModalSubmitInteraction(o, o.at("data").as_object()) {}
cModalSubmitInteraction::cModalSubmitInteraction(const json::object& o, const json::object& d) :
	cInteraction(INTERACTION_MODAL_SUBMIT, o),
	m_custom_id(json::value_to<std::string>(d.at("custom_id"))),
	m_components(json::value_to<std::vector<cActionRow>>(d.at("components"))) {}