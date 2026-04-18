#include "Emoji.h"
#include "Utils.h"
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
	return std::format("{}", *this);
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

std::format_context::iterator
std::formatter<cEmoji>::format(const cEmoji& emoji, std::format_context& ctx) const {
	const auto name = emoji.GetName();
	const auto pId = emoji.GetId();

	if (m_bPct) {
		auto out = ctx.out();
		out = cUtils::PercentEncodeTo(out, name);
		if (pId) {
			auto id = pId->ToString();
			out = cUtils::PercentEncodeTo(out, ":");
			out = std::copy(id.begin(), id.end(), out);
		}
		return out;
	}

	if (pId)
		return std::format_to(ctx.out(), "<{}:{}:{}>", emoji.IsAnimated() ? "a" : "", name, *pId);

	return std::copy(name.begin(), name.end(), ctx.out());
}
