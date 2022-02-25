#pragma once
#ifndef _GREEKBOT_USER_H_
#define _GREEKBOT_USER_H_
#include "Common.h"

class cUser final {
private:
	cSnowflake  id;
	std::string username;
	std::string discriminator;
	std::string avatar;
	bool        bot;
	bool        system;
	// other stuff unimplemented
	
public:
	explicit cUser(const json::value&);
	
	const cSnowflake&  GetId()            const { return id;            }
	const std::string& GetUsername()      const { return username;      }
	const std::string& GetDiscriminator() const { return discriminator; }
	const std::string& GetAvatarUrl()     const { return avatar;        }

	bool IsBotUser()    const { return bot;    }
	bool IsSystemUser() const { return system; }
};
typedef   hHandle<cUser>   hUser;
typedef  chHandle<cUser>  chUser;
typedef  uhHandle<cUser>  uhUser;
typedef uchHandle<cUser> uchUser;
typedef  shHandle<cUser>  shUser;
typedef schHandle<cUser> schUser;

#endif /* _GREEKBOT_USER_H_ */
