#include "Guild.h"
#include "json.h"

cGuild::cGuild(const json::value& v):
	id(json::value_to<cSnowflake>(v.at("id"))),
	name(json::value_to<std::string>(v.at("name"))),
	roles(json::value_to<std::vector<cRole>>(v.at("roles"))) {}

cGuild
tag_invoke(json::value_to_tag<cGuild>, const json::value& v) {
	return cGuild{ v };
}