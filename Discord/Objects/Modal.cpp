#include "Modal.h"
#include <boost/json.hpp>

namespace json = boost::json;

void
tag_invoke(json::value_from_tag, json::value& v, const cModal& modal) {
	json::object& obj = v.emplace_object();
	obj.reserve(3);
	obj.emplace("custom_id", modal.GetCustomId());
	obj.emplace("title", modal.GetTitle());
	json::value_from(modal.GetComponents(), obj["components"]);
}

void
tag_invoke(json::value_from_tag, json::value& v, const cModal::component_type& cmp) {
	std::visit([&v](const auto& cmp) { json::value_from(cmp, v); }, cmp);
}