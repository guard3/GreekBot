#pragma once
#ifndef _GREEKBOT_USER_H_
#define _GREEKBOT_USER_H_
#include "Types.h"
#include "json.h"

class cUser final {
private:
	cSnowflake  id;
	std::string username;
	std::string discriminator;
	char        avatar[100] = "https://cdn.discordapp.com/";
	bool        bot;
	bool        system;
	// other stuff unimplemented
	
public:
	explicit cUser(const json::value&);
	
	chSnowflake GetId()            const { return &id;                   }
	const char* GetUsername()      const { return username.c_str();      }
	const char* GetDiscriminator() const { return discriminator.c_str(); }
	const char* GetAvatarUrl()     const { return avatar;                }

	bool        IsBotUser()        const { return bot;                   }
	bool        IsSystemUser()     const { return system;                }
};
typedef   hHandle<cUser>   hUser;
typedef  chHandle<cUser>  chUser;
typedef  uhHandle<cUser>  uhUser;
typedef uchHandle<cUser> uchUser;
typedef  shHandle<cUser>  shUser;
typedef schHandle<cUser> schUser;

#endif /* _GREEKBOT_USER_H_ */
