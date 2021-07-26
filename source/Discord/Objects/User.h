#pragma once
#ifndef _GREEKBOT_USER_H_
#define _GREEKBOT_USER_H_
#include "json.h"

class cUser final {
private:
	json::string id;
	json::string username;
	json::string discriminator;
	char avatar[100];
	// other stuff unimplemented
	
public:
	cUser(const json::value&);
	
	const char* GetId()            const { return id.c_str();            }
	const char* GetUsername()      const { return username.c_str();      }
	const char* GetDiscriminator() const { return discriminator.c_str(); }
	const char* GetAvatarUrl()     const { return avatar;                }
};
typedef std::shared_ptr<const cUser> hUser;

#endif /* _GREEKBOT_USER_H_ */
