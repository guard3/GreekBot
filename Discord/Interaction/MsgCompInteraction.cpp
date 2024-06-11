#include "MsgCompInteraction.h"
#include <boost/json.hpp>
/* ================================================================================================================== */
using namespace detail;
namespace json = boost::json;
/* ================================================================================================================== */
cMsgCompInteraction::cMsgCompInteraction(const json::value& v) : cMsgCompInteraction(v.as_object()) {}
cMsgCompInteraction::cMsgCompInteraction(const json::object& o) : cMsgCompInteraction(o, o.at("data").as_object()) {}
cMsgCompInteraction::cMsgCompInteraction(const json::object& obj, const json::object& data) :
	cInteraction(INTERACTION_MESSAGE_COMPONENT, obj),
	m_message(obj.at("message")),
	m_custom_id(json::value_to<std::string>(data.at("custom_id"))),
	m_component_type(json::value_to<eComponentType>(data.at("component_type"))),
	m_values([&data] {
		auto p = data.if_contains("values");
		return p ? json::value_to<std::vector<std::string>>(*p) : std::vector<std::string>();
	}()) {}
