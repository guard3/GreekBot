#ifndef DISCORD_MEMBER_H
#define DISCORD_MEMBER_H
#include "MemberFwd.h"
#include "User.h"
#include "Role.h"
#include <vector>
#include <span>
#include <optional>

/* TODO: Maybe make cMemberOptions and cMemberUpdate be the same class? */
class cMemberOptions {
	std::optional<std::string> m_nick;
	std::optional<std::vector<cSnowflake>> m_roles;
	std::optional<std::chrono::sys_time<std::chrono::milliseconds>> m_communications_disabled_until;

public:
	template<typename T = decltype(m_nick)::value_type, typename Arg = T> requires std::constructible_from<T, Arg&&>
	cMemberOptions& SetNick(Arg&& arg) {
		m_nick.emplace(std::forward<Arg>(arg));
		return *this;
	}
	template<typename T = decltype(m_roles)::value_type, typename Arg = T> requires std::constructible_from<T, Arg&&>
	cMemberOptions& SetRoles(Arg&& arg) {
		m_roles.emplace(std::forward<Arg>(arg));
		return *this;
	}
	cMemberOptions& SetCommunicationsDisabledUntil(std::chrono::sys_time<std::chrono::milliseconds> timestamp) {
		m_communications_disabled_until = timestamp;
		return *this;
	}
	template<typename T = decltype(m_roles)::value_type, typename... Args> requires std::constructible_from<T, Args&&...>
	T& EmplaceRoles(Args&&... args) {
		return m_roles.emplace(std::forward<Args>(args)...);
	}

	friend void tag_invoke(boost::json::value_from_tag, boost::json::value&, const cMemberOptions&);
};
void
tag_invoke(boost::json::value_from_tag, boost::json::value&, const cMemberOptions&);

enum eMemberFlag {
	MEMBER_FLAG_NONE                  = 0,
	MEMBER_FLAG_DID_REJOIN            = 1 << 0,
	MEMBER_FLAG_COMPLETED_ONBOARDING  = 1 << 1,
	MEMBER_FLAG_BYPASSES_VERIFICATION = 1 << 2,
	MEMBER_FLAG_STARTED_ONBOARDING    = 1 << 3
};
inline eMemberFlag operator|(eMemberFlag a, eMemberFlag b) noexcept { return (eMemberFlag)((int)a | (int)b); }
inline eMemberFlag operator&(eMemberFlag a, eMemberFlag b) noexcept { return (eMemberFlag)((int)a & (int)b); }
eMemberFlag tag_invoke(boost::json::value_to_tag<eMemberFlag>, const boost::json::value&);

class cMemberUpdate final {
private:
	cUser m_user;
	std::string m_nick;
	std::vector<cSnowflake> m_roles;
	bool m_pending;
	eMemberFlag m_flags;

public:
	explicit cMemberUpdate(const boost::json::value&);
	explicit cMemberUpdate(const boost::json::object&);

	const cUser& GetUser() const noexcept { return m_user; }
	std::string_view GetNickname() const noexcept { return m_nick; }
	std::span<const cSnowflake> GetRoles() const noexcept { return m_roles; }
	eMemberFlag GetFlags() const noexcept { return m_flags; }
	bool IsPending() const noexcept { return m_pending; }
};

class cPartialMember {
public:
	typedef std::chrono::sys_time<std::chrono::milliseconds> time_point;

	explicit cPartialMember(const boost::json::value&);
	explicit cPartialMember(const boost::json::object&);

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
cPartialMember
tag_invoke(boost::json::value_to_tag<cPartialMember>, const boost::json::value&);

class cMember final : public cPartialMember {
private:
	std::optional<cUser> m_user;

public:
	explicit cMember(const boost::json::value&);
	explicit cMember(const boost::json::object&);

	chUser GetUser() const noexcept { return m_user ? &*m_user : nullptr; }
	hUser  GetUser()       noexcept { return m_user ? &*m_user : nullptr; }
};
cMember
tag_invoke(boost::json::value_to_tag<cMember>, const boost::json::value&);
#endif /* DISCORD_MEMBER_H */