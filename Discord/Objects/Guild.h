#ifndef _GREEKBOT_GUILD_H_
#define _GREEKBOT_GUILD_H_
#include "Types.h"
#include "Channel.h"
#include "VoiceState.h"
#include "Role.h"

class cGuild final {
private:
	cSnowflake id;
	std::string name;

	//std::vector<cChannel> channels;
	//std::vector<cVoiceState> voice_states;
public:
	//std::vector<chChannel> Channels;
	//std::vector<chVoiceState> VoiceStates;
	std::vector<cRole> Roles;

	cGuild(const json::value& v) : cGuild(v.as_object()) {}
	cGuild(const json::object& o) : id(o.at("id")), name(o.at("name").as_string().c_str()) {
		auto& a = o.at("roles").as_array();
		Roles.reserve(a.size());
		for (auto& v : a)
			Roles.emplace_back(v);
	}

	chSnowflake GetId() const { return &id; }
	const char* GetName() const { return name.c_str(); }
};
typedef   hHandle<cGuild>   hGuild;
typedef  chHandle<cGuild>  chGuild;
typedef  uhHandle<cGuild>  uhGuild;
typedef uchHandle<cGuild> uchGuild;
typedef  shHandle<cGuild>  shGuild;
typedef schHandle<cGuild> schGuild;
#endif /* _GREEKBOT_GUILD_H_ */
