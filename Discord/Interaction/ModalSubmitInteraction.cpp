#include "ModalSubmitInteraction.h"
#include "json.h"

cModalSubmitInteraction::cModalSubmitInteraction(const json::object& o, const json::object& d) :
	cInteraction(INTERACTION_MODAL_SUBMIT, o),
	m_custom_id(json::value_to<std::string>(d.at("custom_id"))) {
	const json::array& a = d.at("components").as_array();
	for (const json::value& e : a) {
		for (const json::value& m : e.at("components").as_array())
			m_submit.emplace_back(m);
	}
}