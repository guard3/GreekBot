#ifndef GREEKBOT_GUILD_H
#define GREEKBOT_GUILD_H
#include "Role.h"

class cGuild final {
private:
	cSnowflake id;
	std::string name;

public:
	std::vector<cRole> Roles;

	cGuild(const json::value&);
	cGuild(const json::object&);

	const cSnowflake&  GetId()   const { return id;   }
	const std::string& GetName() const { return name; }
};
typedef   hHandle<cGuild>   hGuild;
typedef  chHandle<cGuild>  chGuild;
typedef  uhHandle<cGuild>  uhGuild;
typedef uchHandle<cGuild> uchGuild;
typedef  shHandle<cGuild>  shGuild;
typedef schHandle<cGuild> schGuild;
#endif /* GREEKBOT_GUILD_H */
