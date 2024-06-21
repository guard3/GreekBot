#include "Guild.h"
#include <boost/json.hpp>

namespace json = boost::json;

cGuildCreate::cGuildCreate(const boost::json::value& v): cGuildCreate(v.as_object()) {}
cGuildCreate::cGuildCreate(const boost::json::object& o): m_voice_states(json::value_to<std::vector<cVoiceState>>(o.at("voice_states"))) {}

cGuildCreate
tag_invoke(json::value_to_tag<cGuildCreate>, const json::value& v) {
	return cGuildCreate{ v };
}

cGuild::cGuild(const json::value& v): cGuild(v.as_object()) {}
cGuild::cGuild(const json::object& o):
	id(json::value_to<cSnowflake>(o.at("id"))),
	name(json::value_to<std::string>(o.at("name"))),
	roles(json::value_to<std::vector<cRole>>(o.at("roles"))) {}

cGuild
tag_invoke(json::value_to_tag<cGuild>, const json::value& v) {
	return cGuild{ v };
}