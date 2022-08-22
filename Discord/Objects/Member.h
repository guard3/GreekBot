#pragma once
#ifndef _GREEKBOT_MEMBER_H_
#define _GREEKBOT_MEMBER_H_
#include "User.h"
#include <vector>

class cMember final {
private:
	uhUser user;
	std::string nick, joined_at, premium_since;
	bool deaf, mute;
	// pending
	// permissions

public:
	std::vector<cSnowflake> Roles;

	cMember(const json::object&);
	cMember(const json::value&);
	cMember(const cMember&);
	cMember(cMember&& o) = default;

	cMember& operator=(const cMember&);
	cMember& operator=(cMember&&) = default;

	chUser             GetUser()         const { return user.get();    }
	const std::string& GetNickname()     const { return nick;          }
	const std::string& GetMemberSince()  const { return joined_at;     }
	const std::string& GetPremiumSince() const { return premium_since; }

	bool IsDeaf() const { return deaf; }
	bool IsMute() const { return mute; }
};
typedef   hHandle<cMember>   hMember;
typedef  chHandle<cMember>  chMember;
typedef  uhHandle<cMember>  uhMember;
typedef uchHandle<cMember> uchMember;
typedef  shHandle<cMember>  shMember;
typedef schHandle<cMember> schMember;
#endif /* _GREEKBOT_MEMBER_H_ */