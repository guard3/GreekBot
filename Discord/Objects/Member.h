#ifndef GREEKBOT_MEMBER_H
#define GREEKBOT_MEMBER_H
#include "User.h"
#include "Role.h"
#include <vector>
#include <span>

KW_DECLARE(nick, std::string_view)

/* TODO: Maybe make cMemberOptions and cMemberUpdate be the same class? */
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

class cMemberUpdate final {
private:
	cUser m_user;
	std::string m_nick;
	std::vector<cSnowflake> m_roles;
	bool m_pending;

public:
	explicit cMemberUpdate(const json::value&);

	const cUser& GetUser() const noexcept { return m_user; }
	std::string_view GetNickname() const noexcept { return m_nick; }
	std::span<const cSnowflake> GetRoles() const noexcept { return m_roles; }
	bool IsPending() const noexcept { return m_pending; }
};

enum eMemberFlag {
	MEMBER_FLAG_NONE                  = 0,
	MEMBER_FLAG_DID_REJOIN            = 1 << 0,
	MEMBER_FLAG_COMPLETED_ONBOARDING  = 1 << 1,
	MEMBER_FLAG_BYPASSES_VERIFICATION = 1 << 2,
	MEMBER_FLAG_STARTED_ONBOARDING    = 1 << 3
};
inline eMemberFlag operator|(eMemberFlag a, eMemberFlag b) noexcept { return (eMemberFlag)((int)a | (int)b); }
inline eMemberFlag operator&(eMemberFlag a, eMemberFlag b) noexcept { return (eMemberFlag)((int)a & (int)b); }
eMemberFlag tsg_invoke(json::value_to_tag<eMemberFlag>, const json::value&);

class cPartialMember {
public:
	typedef std::chrono::sys_time<std::chrono::milliseconds> time_point;

	explicit cPartialMember(const json::value&);
	explicit cPartialMember(const json::object&);

	std::string_view   GetNick() const noexcept { return m_nick;        }
	std::string_view GetAvatar() const noexcept { return m_avatar;      }
	eMemberFlag       GetFlags() const noexcept { return m_flags;       }
	ePermission GetPermissions() const noexcept { return m_permissions; }

	std::string   MoveNick() noexcept { return std::move(m_nick);   }
	std::string MoveAvatar() noexcept { return std::move(m_avatar); }

	bool    IsDeaf() const noexcept { return m_deaf;    }
	bool    IsMute() const noexcept { return m_mute;    }
	bool IsPending() const noexcept { return m_pending; }

	time_point JoinedAt() const noexcept { return m_joined_at; }
	time_point PremiumSince() const noexcept { return m_premium_since; }
	time_point CommunicationDisabledUntil() const noexcept { return m_communication_disabled_until; }

	std::span<const cSnowflake> GetRoles() const noexcept { return m_roles;            }
	std::span<cSnowflake>       GetRoles()       noexcept { return m_roles;            }
	std::vector<cSnowflake>    MoveRoles()       noexcept { return std::move(m_roles); }

private:
	std::string m_nick, m_avatar;
	time_point m_joined_at;
	time_point m_premium_since;
	time_point m_communication_disabled_until;
	bool m_deaf, m_mute, m_pending;
	eMemberFlag m_flags;
	ePermission m_permissions;
	std::vector<cSnowflake> m_roles;
};
typedef   hHandle<cPartialMember>   hPartialMember;
typedef  chHandle<cPartialMember>  chPartialMember;
typedef  uhHandle<cPartialMember>  uhPartialMember;
typedef uchHandle<cPartialMember> uchPartialMember;

class cMember final : public cPartialMember {
private:
	std::optional<cUser> m_user;

public:
	explicit cMember(const json::value&);
	explicit cMember(const json::object&);

	chUser GetUser() const noexcept { return m_user ? &*m_user : nullptr; }
	hUser  GetUser()       noexcept { return m_user ? &*m_user : nullptr; }
};
typedef   hHandle<cMember>   hMember;
typedef  chHandle<cMember>  chMember;
typedef  uhHandle<cMember>  uhMember;
typedef uchHandle<cMember> uchMember;

cMember tag_invoke(boost::json::value_to_tag<cMember>, const boost::json::value&);
#endif //GREEKBOT_MEMBER_H