#include "Role.h"
#include "Utils.h"
#include "json.h"

ePermission
tag_invoke(json::value_to_tag<ePermission>, const json::value& v) {
	return (ePermission)cUtils::ParseInt<std::underlying_type_t<ePermission>>(v.as_string());
}

cRole
tag_invoke(json::value_to_tag<cRole>, const json::value& v) {
	return v;
}

cRoleTags::cRoleTags(const json::value& v): cRoleTags(v.as_object()) {}
cRoleTags::cRoleTags(const json::object& o):
	bot_id([&o] {
		auto p = o.if_contains("bot_id");
		return p ? cSnowflake(p->as_string()) : cSnowflake{};
	}()),
	integration_id([&o] {
		auto p = o.if_contains("integration_id");
		return p ? cSnowflake(p->as_string()) : cSnowflake{};
	}()) {}

cRole::cRole(const json::value& v): cRole(v.as_object()) {}
cRole::cRole(const json::object &o):
	m_id(o.at("id").as_string()),
	m_position(o.at("position").to_number<std::size_t>()),
	m_permissions(json::value_to<ePermission>(o.at("permissions"))),
	m_color(json::value_to<cColor>(o.at("color"))),
	m_hoist(o.at("hoist").as_bool()),
	m_managed(o.at("managed").as_bool()),
	m_mentionable(o.at("mentionable").as_bool()),
	m_name(json::value_to<std::string>(o.at("name"))),
	m_icon([&o] {
		auto p = o.if_contains("icon");
		return p && !p->is_null() ? json::value_to<std::string>(*p) : std::string();
	}()),
	m_unicode_emoji([&o] {
		auto p = o.if_contains("unicode_emoji");
		return p && !p->is_null() ? json::value_to<std::string>(*p) : std::string();
	}()) {
	if (auto p = o.if_contains("tags"))
		m_tags.emplace(*p);
}