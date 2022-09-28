#pragma once
#ifndef _GREEKBOT_MEMBER_H_
#define _GREEKBOT_MEMBER_H_
#include "User.h"
#include "Role.h"
#include <vector>
#if 0
enum ePermission : int64_t {
	PERM_NONE                  = 0,
	PERM_CREATE_INSTANT_INVITE = 1 << 0,
	PERM_KICK_MEMBERS          = 1 << 1,
	PERM_BAN_MEMBERS           = 1 << 2,
	PERM_ADMINISTRATOR         = 1 << 3
	// more tba...
};
inline ePermission operator|(ePermission a, ePermission b) { return (ePermission)((int64_t)a | (int64_t)b); }
inline ePermission operator&(ePermission a, ePermission b) { return (ePermission)((int64_t)a | (int64_t)b); }
#endif
class cMember final {
private:
	uhUser user;
	std::string nick, joined_at, premium_since;
	bool deaf, mute;
	// pending
	ePermission permissions;

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
	ePermission        GetPermissions()  const { return permissions;   }

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