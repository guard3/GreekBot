#pragma once
#ifndef _GREEKBOT_USER_H_
#define _GREEKBOT_USER_H_
#include "Types.h"
#include "json.h"

class cUser final {
private:
	cSnowflake  id;
	std::string username;
	int         discriminator;
	char        avatar[100];
	// other stuff unimplemented
	
public:
	cUser(const json::value&);
	
	chSnowflake GetId()            const { return &id;              }
	const char* GetUsername()      const { return username.c_str(); }
	int         GetDiscriminator() const { return discriminator;    }
	const char* GetAvatarUrl()     const { return avatar;           }
};
typedef   hHandle<cUser>   hUser;
typedef  chHandle<cUser>  chUser;
typedef  uhHandle<cUser>  uhUser;
typedef uchHandle<cUser> uchUser;
typedef  shHandle<cUser>  shUser;
typedef schHandle<cUser> schUser;

#endif /* _GREEKBOT_USER_H_ */
