#ifndef GREEKBOT_USER_H
#define GREEKBOT_USER_H
#include "Common.h"

class cUser final {
private:
	cSnowflake  id;
	std::string username;
	std::string avatar;
	std::string global_name;
	bool        bot;
	bool        system;
	// other stuff unimplemented
	
public:
	explicit cUser(const json::object&);
	explicit cUser(const json::value&);

	const cSnowflake&        GetId() const noexcept { return id;          }
	std::string_view   GetUsername() const noexcept { return username;    }
	std::string_view  GetAvatarUrl() const noexcept { return avatar;      }
	std::string_view GetGlobalName() const noexcept { return global_name; }
	bool                 IsBotUser() const noexcept { return bot;         }
	bool              IsSystemUser() const noexcept { return system;      }

	cSnowflake& GetId() noexcept { return id; }

	std::string  MoveUsername() noexcept { return std::move(username); }
	std::string MoveAvatarUrl() noexcept { return std::move(avatar);   }
};
typedef   hHandle<cUser>   hUser;
typedef  chHandle<cUser>  chUser;
typedef  uhHandle<cUser>  uhUser;
typedef uchHandle<cUser> uchUser;
typedef  shHandle<cUser>  shUser;
typedef schHandle<cUser> schUser;
#endif //GREEKBOT_USER_H