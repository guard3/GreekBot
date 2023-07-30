#include "Guild.h"
#include "json.h"

cGuild::cGuild(const json::object &o):
	id(json::value_to<cSnowflake>(o.at("id"))),
	name(json::value_to<std::string>(o.at("name"))) {
	auto& a = o.at("roles").as_array();
	Roles.reserve(a.size());
	for (auto& v : a)
		Roles.emplace_back(v);
}

cGuild::cGuild(const json::value &v) : cGuild(v.as_object()) {}