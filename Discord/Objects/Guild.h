#ifndef GREEKBOT_GUILD_H
#define GREEKBOT_GUILD_H
#include "Role.h"
#include "VoiceState.h"
#include <vector>
#include <span>

class cGuildCreate final {
	std::vector<cVoiceState> m_voice_states;

public:
	explicit cGuildCreate(const boost::json::value&);
	explicit cGuildCreate(const boost::json::object&);

	std::span<const cVoiceState> GetVoiceStates() const noexcept { return m_voice_states; }
	std::vector<cVoiceState> MoveVoiceStates() noexcept { return std::move(m_voice_states); }
};
using   hGuildCreate =   hHandle<cGuildCreate>;
using  chGuildCreate =  chHandle<cGuildCreate>;
using  uhGuildCreate =  uhHandle<cGuildCreate>;
using uchGuildCreate = uchHandle<cGuildCreate>;

cGuildCreate
tag_invoke(boost::json::value_to_tag<cGuildCreate>, const boost::json::value&);

class cGuild final {
private:
	cSnowflake id;
	std::string name;
	std::vector<cRole> roles;

public:
	explicit cGuild(const boost::json::value&);
	explicit cGuild(const boost::json::object&);

	const cSnowflake&      GetId()    const noexcept { return id;    }
	const std::string&     GetName()  const noexcept { return name;  }
	std::span<const cRole> GetRoles() const noexcept { return roles; }

	std::vector<cRole> MoveRoles() noexcept { return std::move(roles); }
};
typedef   hHandle<cGuild>   hGuild;
typedef  chHandle<cGuild>  chGuild;
typedef  uhHandle<cGuild>  uhGuild;
typedef uchHandle<cGuild> uchGuild;

cGuild
tag_invoke(boost::json::value_to_tag<cGuild>, const boost::json::value&);
#endif /* GREEKBOT_GUILD_H */