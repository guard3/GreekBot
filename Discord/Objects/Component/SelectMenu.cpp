#include "SelectMenu.h"
#include <boost/json.hpp>
/* ================================================================================================================== */
namespace json = boost::json;
/* ================================================================================================================== */
cSelectMenu::cSelectMenu(const json::value& v) : cSelectMenu(v.as_object()) {}
cSelectMenu::cSelectMenu(const json::object& o):
	m_custom_id(json::value_to<std::string>(o.at("custom_id"))),
	m_placeholder([&o] {
		auto p = o.if_contains("placeholder");
		return p ? json::value_to<std::string>(*p) : std::string();
	}()),
	m_options(json::value_to<std::vector<cSelectOption>>(o.at("options"))){
}
/* ================================================================================================================== */
cSelectMenu
tag_invoke(json::value_to_tag<cSelectMenu>, const json::value& v) {
	return cSelectMenu{ v };
}
/* ================================================================================================================== */
cSelectOption::cSelectOption(const json::value& v) : cSelectOption(v.as_object()) {}
cSelectOption::cSelectOption(const json::object& o):
	m_label(json::value_to<std::string>(o.at("label"))),
	m_value(json::value_to<std::string>(o.at("value"))),
	m_description([&o] {
		auto p = o.if_contains("description");
		return p ? json::value_to<std::string>(*p) : std::string();
	}()) {
	if (auto p = o.if_contains("emoji"))
		m_emoji.emplace(*p);
}
/* ================================================================================================================== */
cSelectOption
tag_invoke(json::value_to_tag<cSelectOption>, const json::value& v) {
	return cSelectOption{ v };
}
void
tag_invoke(json::value_from_tag, json::value& v, const cSelectOption& so) {
	json::object& obj = v.emplace_object();
	obj.reserve(4);
	obj.emplace("label", so.GetLabel());
	obj.emplace("value", so.GetValue());
	if (auto d = so.GetDescription(); !d.empty())
		obj.emplace("description", d);
	if (auto e = so.GetEmoji().Get(); e)
		json::value_from(*e, obj["emoji"]);
}