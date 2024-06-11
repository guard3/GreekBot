#include "Modal.h"
#include <boost/json.hpp>

namespace json = boost::json;

void
tag_invoke(json::value_from_tag, json::value& v, const cModal& modal) {
	json::object& obj = v.emplace_object();
	obj.reserve(3);
	obj.emplace("custom_id", modal.m_custom_id);
	obj.emplace("title", modal.m_title);
	json::value_from(modal.m_components, obj["components"]);
}