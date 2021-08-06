#pragma once
#ifndef _GREEKBOT_MEMBER_H_
#define _GREEKBOT_MEMBER_H_
#include "Types.h"
#include "json.h"
#include "User.h"
#include <vector>

class cMember final {
private:
	uchUser user;
	uchHandle<std::string> nick;
	std::vector<cSnowflake> roles;
	// joined_at
	// premium_since
	// deaf
	// mute
	// pending
	// permissions
	
public:
	const std::vector<chSnowflake> Roles;
	
	cMember(const json::value& v);
	
	chUser      GetUser()     const { return user.get(); }
	const char* GetNickname() const { return nick ? nick->c_str() : nullptr; }
};
typedef   hHandle<cMember>   hMember;
typedef  chHandle<cMember>  chMember;
typedef  uhHandle<cMember>  uhMember;
typedef uchHandle<cMember> uchMember;
typedef  shHandle<cMember>  shMember;
typedef schHandle<cMember> schMember;

#endif /* _GREEKBOT_MEMBER_H_ */
