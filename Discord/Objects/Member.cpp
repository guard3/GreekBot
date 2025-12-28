#include "Member.h"
#include "Utils.h"
#include <boost/json.hpp>

namespace json = boost::json;

eMemberFlag
tag_invoke(json::value_to_tag<eMemberFlag>, const json::value& v) {
	return static_cast<eMemberFlag>(v.to_number<int>());
}

cMemberUpdate::cMemberUpdate(const json::value& v) : cMemberUpdate(v.as_object()) {}
cMemberUpdate::cMemberUpdate(const json::object& o) :
	m_user{ o.at("user") },
	m_roles{ json::value_to<std::vector<cSnowflake>>(o.at("roles")) } {
	const json::value* p;
	if ((p = o.if_contains("nick"))) {
		auto result = json::try_value_to<std::string>(*p);
		if (result.has_value())
			m_nick = std::move(result.value());
	}
	m_pending = (p = o.if_contains("pending")) && p->as_bool();
	m_flags = (p = o.if_contains("flags")) ? json::value_to<eMemberFlag>(*p) : MEMBER_FLAG_NONE;
}

cPartialMember::cPartialMember(const json::value& v): cPartialMember(v.as_object()) {}
cPartialMember::cPartialMember(const json::object& o):
	m_nick([&o] {
		auto p = o.if_contains("nick");
		return p && !p->is_null() ? json::value_to<std::string>(*p) : std::string();
	}()),
	m_avatar([&o] {
		auto p = o.if_contains("avatar");
		return p && !p->is_null() ? json::value_to<std::string>(*p) : std::string();
	}()),
	m_joined_at(cUtils::ParseISOTimestamp(o.at("joined_at").as_string())),
	m_premium_since([&o] {
		auto p = o.if_contains("premium_since");
		return p && !p->is_null() ? cUtils::ParseISOTimestamp(p->as_string()) : time_point{};
	}()),
	m_communication_disabled_until([&o] {
		auto p = o.if_contains("communication_disabled_until");
		return p && !p->is_null() ? cUtils::ParseISOTimestamp(p->as_string()) : time_point{};
	}()),
	m_deaf([&o] {
		auto p = o.if_contains("deaf");
		return p && p->as_bool();
	}()),
	m_mute([&o] {
		auto p = o.if_contains("mute");
		return p && p->as_bool();
	}()),
	m_pending([&o] {
		auto p = o.if_contains("pending");
		return p && p->as_bool();
	}()),
	m_flags(json::value_to<eMemberFlag>(o.at("flags"))),
	m_permissions([&o] {
		auto p = o.if_contains("permissions");
		return p ? json::value_to<ePermission>(*p) : ePermission{};
	}()),
	m_roles(json::value_to<std::vector<cSnowflake>>(o.at("roles"))) {}

cPartialMember
tag_invoke(json::value_to_tag<cPartialMember>, const json::value& v) {
	return cPartialMember{ v };
}

cMember::cMember(const json::value& v) : cMember(v.as_object()) {}
cMember::cMember(const json::object& o) : cPartialMember(o) {
	if (auto p = o.if_contains("user"))
		m_user.emplace(*p);
}

cMember
tag_invoke(json::value_to_tag<cMember>, const json::value& v) {
	return cMember{ v };
}
void
tag_invoke(json::value_from_tag, json::value& v, const cMemberOptions& m) {
	json::object& obj = v.emplace_object();
	obj.reserve(3);
	if (m.m_nick)
		obj.emplace("nick", *m.m_nick);
	if (m.m_roles)
		json::value_from(*m.m_roles, obj["roles"]);
	if (m.m_communications_disabled_until) {
		auto& cdu = obj["communication_disabled_until"];
		if (*m.m_communications_disabled_until != std::chrono::sys_time<std::chrono::milliseconds>{})
			cdu = cUtils::FormatISOTimestamp(*m.m_communications_disabled_until);
	}
}