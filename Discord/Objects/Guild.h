#ifndef GREEKBOT_GUILD_H
#define GREEKBOT_GUILD_H
#include "Role.h"
#include <span>

class cGuild final {
private:
	cSnowflake id;
	std::string name;
	std::vector<cRole> roles;

public:
	explicit cGuild(const boost::json::value&);

	const cSnowflake&      GetId()    const noexcept { return id;    }
	const std::string&     GetName()  const noexcept { return name;  }
	std::span<const cRole> GetRoles() const noexcept { return roles; }

	std::vector<cRole> MoveRoles() noexcept { return std::move(roles); }
};
typedef   hHandle<cGuild>   hGuild;
typedef  chHandle<cGuild>  chGuild;
typedef  uhHandle<cGuild>  uhGuild;
typedef uchHandle<cGuild> uchGuild;

cGuild tag_invoke(boost::json::value_to_tag<cGuild>, const boost::json::value&);
#endif /* GREEKBOT_GUILD_H */
