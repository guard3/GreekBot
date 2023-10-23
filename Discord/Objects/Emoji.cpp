#include "Emoji.h"
#include "json.h"
#include <fmt/format.h>

cEmoji::cEmoji(const json::value& v): m_animated(false) {
	auto& o = v.as_object();
	auto name = json::try_value_to<std::string>(o.at("name"));
	if (name.has_value())
		m_name = std::move(name.value());
	auto id = json::try_value_to<std::string_view>(o.at("id"));
	if (id.has_value())
		m_id.emplace(id.value());
	if (auto p = o.if_contains("animated"))
		m_animated = p && p->as_bool();
}

std::string
cEmoji::ToString() const {
	if (m_id.has_value())
		return fmt::format("<{}:{}:{}>", m_animated ? "a" : "", m_name, *m_id);
	return m_name;
}
void
tag_invoke(const json::value_from_tag&, json::value& v, const cEmoji& e) {
	auto& o = v.emplace_object();
	if (!e.GetName().empty())
		o.emplace("name", e.GetName());
	if (e.GetId())
		o.emplace("id", e.GetId()->ToString());
	else
		o.emplace("id", nullptr);
	o.emplace("animated", e.IsAnimated());
}