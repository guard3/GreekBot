#include "Member.h"
#include "json.h"

cMember::cMember(const json::value& v):
	m_joined_at(cUtils::ParseTimestamp(json::value_to<std::string>(v.at("joined_at")))),
	m_roles(json::value_to<std::vector<cSnowflake>>(v.at("roles"))) {
	auto& o = v.as_object();
	const json::value* p;
	if ((p = o.if_contains("user")))
		m_user.emplace(*p);
	if ((p = o.if_contains("nick"))) {
		auto result = json::try_value_to<std::string>(*p);
		if (result.has_value())
			m_nick = std::move(result.value());
	}
	if ((p = o.if_contains("premium_since"))) {
		if (auto s = p->if_string())
			m_premium_since = cUtils::ParseTimestamp(s->c_str());
	}
	m_permissions = (p = o.if_contains("permissions")) ? json::value_to<ePermission>(*p) : PERM_NONE;
	m_deaf = (p = o.if_contains("deaf")) && p->as_bool();
	m_mute = (p = o.if_contains("mute")) && p->as_bool();
}

cMember
tag_invoke(json::value_to_tag<cMember>, const json::value& v) {
	return cMember{ v };
}