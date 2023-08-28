#include "Member.h"
#include "Utils.h"
#include "json.h"

cPartialMember::cPartialMember(const json::value& v):
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

cMember::cMember(const json::value& v):
	m_joined_at(cUtils::ParseTimestamp(json::value_to<std::string_view>(v.at("joined_at")))),
	m_roles(json::value_to<std::vector<cSnowflake>>(v.at("roles"))) {
	auto& o = v.as_object();
	const json::value* p;
	if ((p = o.if_contains("user")))
		m_user.emplace(*p);
	if ((p = o.if_contains("nick"))) {
		if (auto result = json::try_value_to<std::string>(*p); result.has_value())
			m_nick = std::move(result.value());
	}
	if ((p = o.if_contains("premium_since"))) {
		if (auto result = json::try_value_to<std::string_view>(*p); result.has_value())
			m_premium_since = cUtils::ParseTimestamp(result.value());
	}
	m_permissions = (p = o.if_contains("permissions")) ? json::value_to<ePermission>(*p) : PERM_NONE;
	m_deaf = (p = o.if_contains("deaf")) && p->as_bool();
	m_mute = (p = o.if_contains("mute")) && p->as_bool();
	m_pending = (p = o.if_contains("pending")) && p->as_bool();
}

cMember
tag_invoke(json::value_to_tag<cMember>, const json::value& v) {
	return cMember{ v };
}