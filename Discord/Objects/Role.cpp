#include "Role.h"
#include "Utils.h"
#include "json.h"

// TODO: add this to cCDN
#define DISCORD_IMAGE_BASE_URL "https://cdn.discordapp.com/"

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

cRole::cRole(const json::value& v) : cRole(v.as_object()) {}
cRole::cRole(const json::object &o) :
	id(json::value_to<cSnowflake>(o.at("id"))),
	name(json::value_to<std::string>(o.at("name"))),
	color(json::value_to<cColor>(o.at("color"))),
	hoist(o.at("hoist").as_bool()),
	position(o.at("position").to_number<int>()),
	permissions(json::value_to<ePermission>(o.at("permissions"))),
	managed(o.at("managed").as_bool()),
	mentionable(o.at("mentionable").as_bool()),
	unicode_emoji([&o] {
		auto p = o.if_contains("unicode_emoji");
		return p && !p->is_null() ? json::value_to<std::string>(*p) : std::string();
	}()) {
	if (auto p = o.if_contains("icon")) {
		if (auto s = p->if_string()) {
			icon = fmt::format("{}role-icons/{}/{}.{}?size=4096", DISCORD_IMAGE_BASE_URL, id, s->c_str(), s->starts_with("a_") ? "gif" : "png");
		}
	}
	if (auto p = o.if_contains("tags"))
		tags.emplace(*p);
}