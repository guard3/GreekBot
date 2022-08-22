#include "Guild.h"
#include "json.h"

cGuild::cGuild(const json::object &o) : id(o.at("id")), name(o.at("name").as_string().c_str()) {
	auto& a = o.at("roles").as_array();
	Roles.reserve(a.size());
	for (auto& v : a)
		Roles.emplace_back(v);
}

cGuild::cGuild(const json::value &v) : cGuild(v.as_object()) {}