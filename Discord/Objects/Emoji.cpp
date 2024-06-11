#include "Emoji.h"
#include <boost/json.hpp>

namespace json = boost::json;

cEmoji::cEmoji(const json::value& v) : cEmoji(v.as_object()) {}
cEmoji::cEmoji(const json::object& o) :
	m_id([&o] {
		auto p = o.if_contains("id");
		return p && !p->is_null() ? json::value_to<cSnowflake>(*p) : cSnowflake{};
	}()),
	m_name([&o] {
		auto& v = o.at("name");
		return v.is_null() ? std::string() : json::value_to<std::string>(v);
	}()),
	m_animated([&o] {
		auto p = o.if_contains("animated");
		return p && p->as_bool();
	}()) {}

std::string
cEmoji::ToString() const {
	if (m_id.ToInt())
		return fmt::format("<{}:{}:{}>", m_animated ? "a" : "", m_name, m_id);
	return m_name;
}
cEmoji
tag_invoke(json::value_to_tag<cEmoji>, const json::value& v) {
	return cEmoji{ v };
}
void
tag_invoke(json::value_from_tag, json::value& v, const cEmoji& e) {
	auto& obj = v.emplace_object();
	obj.emplace("name", e.GetName());
	if (auto pId = e.GetId().Get())
		json::value_from(*pId, obj["id"]);
	else
		obj.emplace("id", nullptr);
	if (e.IsAnimated())
		obj.emplace("animated", true);
}