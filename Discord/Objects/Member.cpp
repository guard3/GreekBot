#include "Member.h"
#include "Utils.h"
#include "json.h"

eMemberFlag
tag_invoke(json::value_to_tag<eMemberFlag>, const json::value& v) {
	return static_cast<eMemberFlag>(v.to_number<int>());
}

cMemberUpdate::cMemberUpdate(const json::value& v):
	m_user{ v.at("user") },
	m_roles{ json::value_to<std::vector<cSnowflake>>(v.at("roles")) } {
	auto& o = v.as_object();
	const json::value* p;
	if ((p = o.if_contains("nick"))) {
		auto result = json::try_value_to<std::string>(*p);
		if (result.has_value())
			m_nick = std::move(result.value());
	}
	m_pending = (p = o.if_contains("pending")) && p->as_bool();
}

cPartialMember::cPartialMember(const json::value& v): cPartialMember(v.as_object()) {}
cPartialMember::cPartialMember(const json::object& o):
	m_roles(json::value_to<std::vector<cSnowflake>>(o.at("roles"))),
	m_joined_at(cUtils::ParseTimestamp<std::chrono::milliseconds>(json::value_to<std::string>(o.at("joined_at")))),
	m_flags(json::value_to<eMemberFlag>(o.at("flags"))) {
	const json::value* p;
	if ((p = o.if_contains("nick"))) {
		if (!p->is_null())
			m_nick = json::value_to<std::string>(*p);
	}
	if ((p = o.if_contains("avatar"))) {
		if (!p->is_null())
			m_avatar = json::value_to<std::string>(*p);
	}
	if ((p = o.if_contains("premium_since"))) {
		if (!p->is_null())
			m_premium_since = cUtils::ParseTimestamp<std::chrono::milliseconds>(json::value_to<std::string>(*p));
	}
	if ((p = o.if_contains("communication_disabled_until"))) {
		if (!p->is_null())
			m_communication_disabled_until = cUtils::ParseTimestamp<std::chrono::milliseconds>(json::value_to<std::string>(*p));
	}
	m_permissions = (p = o.if_contains("permissions")) ? json::value_to<ePermission>(*p) : PERM_NONE;
	m_deaf = (p = o.if_contains("deaf")) && p->as_bool();
	m_mute = (p = o.if_contains("mute")) && p->as_bool();
	m_pending = (p = o.if_contains("pending")) && p->as_bool();
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
tag_invoke(const json::value_from_tag&, json::value& v, const cMemberOptions& m) {
	json::object& obj = v.emplace_object();
	if (m.m_nick) {
		if (m.m_nick->empty())
			obj.emplace("nick", nullptr);
		else
			obj.emplace("nick", *m.m_nick);
	}
}