#ifndef DISCORD_GUILD_H
#define DISCORD_GUILD_H
#include "GuildFwd.h"
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

	/**
	 * Allow implicit conversion to crefGuild
	 */
	operator crefGuild() const noexcept { return id; }
};

cGuild
tag_invoke(boost::json::value_to_tag<cGuild>, const boost::json::value&);
#endif /* DISCORD_GUILD_H */