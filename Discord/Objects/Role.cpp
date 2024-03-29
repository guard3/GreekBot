#include "Role.h"
#include "Utils.h"
#include "json.h"

// TODO: add this to cCDN
#define DISCORD_IMAGE_BASE_URL "https://cdn.discordapp.com/"

ePermission
tag_invoke(json::value_to_tag<ePermission>, const json::value& v) {
	return static_cast<ePermission>(cUtils::ParseInt<uint64_t>(json::value_to<std::string_view>(v)));
}

cRole
tag_invoke(json::value_to_tag<cRole>, const json::value& v) {
	return v;
}

cRoleTags::cRoleTags(const json::object &o) {
	if (auto p = o.if_contains("bot_id"))
		bot_id = cHandle::MakeUnique<cSnowflake>(json::value_to<cSnowflake>(*p));
	if (auto p = o.if_contains("integration_id"))
		integration_id = cHandle::MakeUnique<cSnowflake>(json::value_to<cSnowflake>(*p));
}

cRoleTags::cRoleTags(const json::value& v) : cRoleTags(v.as_object()) {}

cRoleTags::cRoleTags(const cRoleTags &o) {
	if (o.bot_id) bot_id = cHandle::MakeUnique<cSnowflake>(*o.bot_id);
	if (o.integration_id) integration_id = cHandle::MakeUnique<cSnowflake>(*o.integration_id);
}

cRoleTags&
cRoleTags::operator=(cRoleTags o) {
	std::swap(bot_id,         o.bot_id        );
	std::swap(integration_id, o.integration_id);
	return *this;
}


cRole::cRole(const json::object &o) :
	id(json::value_to<cSnowflake>(o.at("id"))),
	name(json::value_to<std::string>(o.at("name"))),
	color(json::value_to<cColor>(o.at("color"))),
	hoist(o.at("hoist").as_bool()),
	position(o.at("position").to_number<int>()),
	permissions(json::value_to<ePermission>(o.at("permissions"))),
	managed(o.at("managed").as_bool()),
	mentionable(o.at("mentionable").as_bool())
{
	if (auto p = o.if_contains("icon")) {
		if (auto s = p->if_string()) {
			icon = fmt::format("{}role-icons/{}/{}.{}?size=4096", DISCORD_IMAGE_BASE_URL, id, s->c_str(), s->starts_with("a_") ? "gif" : "png");
		}
	}
	if (auto p = o.if_contains("unicode_emoji")) {
		if (auto s = p->if_string())
			unicode_emoji = s->c_str();
	}
	if (auto p = o.if_contains("tags"))
		tags = cHandle::MakeUnique<cRoleTags>(*p);
}

cRole::cRole(const json::value& v) : cRole(v.as_object()) {}

cRole::cRole(const cRole& o) :
	id(o.id),
	name(o.name),
	color(o.color),
	hoist(o.hoist),
	icon(o.icon),
	unicode_emoji(o.unicode_emoji),
	position(o.position),
	permissions(o.permissions),
	managed(o.managed),
	mentionable(o.mentionable)
{
	if (o.tags) cHandle::MakeUnique<cRoleTags>(*o.tags);
}

cRole&
cRole::operator=(cRole o) {
	std::swap(name, o.name);
	std::swap(icon, o.icon);
	std::swap(tags, o.tags);
	std::swap(unicode_emoji, o.unicode_emoji);
	id          = o.id;
	color       = o.color;
	hoist       = o.hoist;
	position    = o.position;
	permissions = o.permissions;
	managed     = o.managed;
	mentionable = o.mentionable;
	return *this;
}