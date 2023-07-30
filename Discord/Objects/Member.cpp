#include "Member.h"
#include "json.h"

cMember::cMember(const json::object& o):
	m_joined_at(cUtils::ParseTimestamp(json::value_to<std::string>(o.at("joined_at")))),
	m_roles(json::value_to<std::vector<cSnowflake>>(o.at("roles")))
	{
	//auto& a = o.at("roles").as_array();
	//m_roles.reserve(a.size());
	//for (auto& v : a)
	//	m_roles.emplace_back(v);
	const json::value* v;
	if ((v = o.if_contains("user")))
		m_user.emplace(*v);
	if ((v = o.if_contains("nick"))) {
		if (auto s = v->if_string())
			m_nick = s->c_str();
	}
	if ((v = o.if_contains("premium_since"))) {
		if (auto s = v->if_string())
			m_premium_since = cUtils::ParseTimestamp(s->c_str());
	}
	m_permissions = (v = o.if_contains("permissions")) ? json::value_to<ePermission>(*v) : PERM_NONE;
	m_deaf = (v = o.if_contains("deaf")) && v->as_bool();
	m_mute = (v = o.if_contains("mute")) && v->as_bool();
}

cMember::cMember(const json::value& v) : cMember(v.as_object()) {}