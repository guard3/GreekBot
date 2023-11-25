#ifndef GREEKBOT_USER_H
#define GREEKBOT_USER_H
#include "Common.h"

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
	explicit cUser(const json::object&);
	explicit cUser(const json::value&);

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
typedef   hHandle<cUser>   hUser;
typedef  chHandle<cUser>  chUser;
typedef  uhHandle<cUser>  uhUser;
typedef uchHandle<cUser> uchUser;

class crefUser final {
private:
	struct user_ref;
	/* The type-erased pointer to user object */
	const void* m_value;
	/* Manual virtual functions */
	const cSnowflake&(*m_fId)(const void*) noexcept;
	std::string_view(*m_fAvatar)(const void*) noexcept;
	std::uint16_t(*m_fDiscriminator)(const void*) noexcept;
	/* Constructor for a plain id-hash-discriminator group */
	crefUser(const user_ref&) noexcept;
public:
	crefUser(const cUser& user) noexcept;
	crefUser(const cSnowflake& id, std::string_view hash = {}, std::uint16_t discr = 0) noexcept;

	const cSnowflake&        GetId() const noexcept { return m_fId(m_value);            }
	std::string_view     GetAvatar() const noexcept { return m_fAvatar(m_value);        }
	std::uint16_t GetDiscriminator() const noexcept { return m_fDiscriminator(m_value); }
};
#endif //GREEKBOT_USER_H