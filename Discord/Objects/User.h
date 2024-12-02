#ifndef DISCORD_USER_H
#define DISCORD_USER_H
#include "Base.h"
#include "UserFwd.h"

class cUser final {
private:
	cSnowflake    m_id;
	std::string   m_username;
	std::string   m_avatar;
	std::string   m_global_name;
	std::uint16_t m_discriminator;
	bool          m_bot;
	bool          m_system;
	// other stuff unimplemented
	
public:
	explicit cUser(const boost::json::object&);
	explicit cUser(const boost::json::value&);

	const cSnowflake&        GetId() const noexcept { return m_id;            }
	std::string_view   GetUsername() const noexcept { return m_username;      }
	std::string_view     GetAvatar() const noexcept { return m_avatar;        }
	std::string_view GetGlobalName() const noexcept { return m_global_name;   }
	uint16_t      GetDiscriminator() const noexcept { return m_discriminator; }
	bool                 IsBotUser() const noexcept { return m_bot;           }
	bool              IsSystemUser() const noexcept { return m_system;        }

	cSnowflake& GetId() noexcept { return m_id; }

	std::string MoveUsername() noexcept { return std::move(m_username); }
	std::string   MoveAvatar() noexcept { return std::move(m_avatar);   }
};
/* ================================================================================================================== */
inline crefUser::crefUser(const cUser& user) noexcept : m_id(user.GetId()) {}
#endif /* DISCORD_USER_H */