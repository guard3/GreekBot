#ifndef GREEKBOT_MEMBER_H
#define GREEKBOT_MEMBER_H
#include "User.h"
#include "Role.h"
#include <vector>
#include <span>

KW_DECLARE(nick, std::string_view)

class cMemberOptions {
private:
	std::optional<std::string> m_nick;

	template<kw::key... Keys>
	cMemberOptions(kw::pack<Keys...> pack) {
		if (auto p = kw::get_if<"nick">(pack))
			m_nick.emplace(p->begin(), p->end());
	}
public:
	template<kw::key... Keys>
	cMemberOptions(kw::arg<Keys>&... kwargs): cMemberOptions(kw::pack{ kwargs... }) {}

	friend void tag_invoke(const json::value_from_tag&, json::value&, const cMemberOptions&);
};

void tag_invoke(const json::value_from_tag&, json::value&, const cMemberOptions&);

/* TODO: Make cMember derive from cPartialMember */
class cPartialMember {
private:
	cUser m_user;
	std::string m_nick;
	std::vector<cSnowflake> m_roles;
	bool m_pending;

public:
	explicit cPartialMember(const json::value&);

	const cUser& GetUser() const noexcept { return m_user; }
	std::string_view GetNickname() const noexcept { return m_nick; }
	std::span<const cSnowflake> GetRoles() const noexcept { return m_roles; }
	bool IsPending() const noexcept { return m_pending; }
};

class cMember final {
private:
	std::optional<cUser> m_user;
	std::string m_nick;
	bool m_deaf, m_mute;
	// ...
	ePermission m_permissions;
	std::chrono::sys_seconds m_joined_at, m_premium_since;
	std::vector<cSnowflake> m_roles;
	bool m_pending;

public:
	explicit cMember(const json::value&);

	hUser  GetUser()     noexcept { return m_user.has_value() ? &*m_user : nullptr; }
	std::string& GetNickname() noexcept { return m_nick; }
	std::vector<cSnowflake>& GetRoles() noexcept { return m_roles; }

	chUser  GetUser()         const noexcept { return const_cast<cMember*>(this)->GetUser();     }
	const std::string& GetNickname()     const noexcept { return const_cast<cMember*>(this)->GetNickname(); }
	const std::vector<cSnowflake>& GetRoles() const noexcept { return const_cast<cMember*>(this)->GetRoles(); }
	ePermission        GetPermissions()  const noexcept { return m_permissions;   }

	std::chrono::sys_seconds JoinedAt()     const noexcept { return m_joined_at;     }
	std::chrono::sys_seconds PremiumSince() const noexcept { return m_premium_since; }

	bool IsDeaf() const noexcept { return m_deaf; }
	bool IsMute() const noexcept { return m_mute; }
	bool IsPending() const noexcept { return m_pending; }
};
typedef   hHandle<cMember>   hMember;
typedef  chHandle<cMember>  chMember;
typedef  uhHandle<cMember>  uhMember;
typedef uchHandle<cMember> uchMember;
typedef  shHandle<cMember>  shMember;
typedef schHandle<cMember> schMember;

cMember tag_invoke(boost::json::value_to_tag<cMember>, const boost::json::value&);
#endif //GREEKBOT_MEMBER_H