#include "Component/ComponentType.h"
#include "ModalSubmitInteraction.h"
#include <boost/json.hpp>
/* ================================================================================================================== */
using namespace detail;
namespace json = boost::json;
/* ================================================================================================================== */
cModalSubmitInteraction::cModalSubmitInteraction(const json::value& v) : cModalSubmitInteraction(v.as_object()) {}
cModalSubmitInteraction::cModalSubmitInteraction(const json::object& o) : cModalSubmitInteraction(o, o.at("data").as_object()) {}
cModalSubmitInteraction::cModalSubmitInteraction(const json::object& o, const json::object& d) :
	cInteraction(INTERACTION_MODAL_SUBMIT, o),
	m_custom_id(json::value_to<std::string>(d.at("custom_id"))),
	m_components(json::value_to<std::vector<component_type>>(d.at("components"))) {}

cModalSubmitInteraction::component_type
tag_invoke(json::value_to_tag<cModalSubmitInteraction::component_type>, const json::value& v) {
	switch (v.at("type").to_number<int>()) {
	case COMPONENT_ACTION_ROW:
		return cModalSubmitInteraction::component_type(std::in_place_type<cActionRow>, v);
	case COMPONENT_LABEL:
		return cModalSubmitInteraction::component_type(std::in_place_type<cPartialLabel>, v);
	default:
		return cModalSubmitInteraction::component_type(std::in_place_type<cUnsupportedComponent>, v);
	}
}