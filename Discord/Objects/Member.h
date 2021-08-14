#pragma once
#ifndef _GREEKBOT_MEMBER_H_
#define _GREEKBOT_MEMBER_H_
#include "Types.h"
#include "User.h"
#include <vector>

class cMember final {
private:
	hUser user;
	char* nick;
	// joined_at
	// premium_since
	// deaf
	// mute
	// pending
	// permissions
	
public:
	const std::vector<chSnowflake> Roles;
	
	explicit cMember(const json::value&);
	cMember(const cMember&);
	cMember(cMember&& o) noexcept;
	~cMember();
	
	[[nodiscard]] chUser      GetUser()     const { return user; }
	[[nodiscard]] const char* GetNickname() const { return nick; }
};
typedef   hHandle<cMember>   hMember;
typedef  chHandle<cMember>  chMember;
typedef  uhHandle<cMember>  uhMember;
typedef uchHandle<cMember> uchMember;
#endif /* _GREEKBOT_MEMBER_H_ */