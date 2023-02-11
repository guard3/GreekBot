#ifndef GREEKBOT_MEMBER_H
#define GREEKBOT_MEMBER_H
#include "User.h"
#include "Role.h"
#include <vector>

class cMember final {
private:
	std::optional<cUser> m_user;
	std::string m_nick;
	bool m_deaf, m_mute;
	// ...
	ePermission m_permissions;
	chrono::sys_seconds m_joined_at, m_premium_since;
	std::vector<cSnowflake> m_roles;

public:
	explicit cMember(const json::object&);
	explicit cMember(const json::value&);

	hUser  GetUser()     noexcept { return m_user.has_value() ? &*m_user : nullptr; }
	std::string& GetNickname() noexcept { return m_nick; }
	std::vector<cSnowflake>& GetRoles() noexcept { return m_roles; }

	chUser  GetUser()         const noexcept { return const_cast<cMember*>(this)->GetUser();     }
	const std::string& GetNickname()     const noexcept { return const_cast<cMember*>(this)->GetNickname(); }
	const std::vector<cSnowflake>& GetRoles() const noexcept { return const_cast<cMember*>(this)->GetRoles(); }
	ePermission        GetPermissions()  const noexcept { return m_permissions;   }

	chrono::sys_seconds JoinedAt()     const noexcept { return m_joined_at;     }
	chrono::sys_seconds PremiumSince() const noexcept { return m_premium_since; }

	bool IsDeaf() const noexcept { return m_deaf; }
	bool IsMute() const noexcept { return m_mute; }
};
typedef       cPtr<cMember>  hMember;
typedef cPtr<const cMember> chMember;
typedef  uhHandle<cMember>  uhMember;
typedef uchHandle<cMember> uchMember;
typedef  shHandle<cMember>  shMember;
typedef schHandle<cMember> schMember;
#endif //GREEKBOT_MEMBER_H