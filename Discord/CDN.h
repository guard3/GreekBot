#ifndef DISCORD_CDN_H
#define DISCORD_CDN_H
#include "Base.h"
#include "RoleFwd.h"
#include "UserFwd.h"

enum class eImageFormat {
	PNG,
	JPG,
	JPEG,
	WEBP
};

class cCDN final {
public:
	cCDN() = delete;

	static std::string GetDefaultUserAvatar(const cUser& user);
	static std::string GetDefaultUserAvatar(const cSnowflake& user_id, std::uint16_t discriminator = 0);
	static std::string GetUserAvatar(const cUser& user, eImageFormat img = eImageFormat::PNG, std::size_t size = 4096);
	static std::string GetUserAvatar(const cSnowflake& user_id, std::string_view hash = {}, std::uint16_t discriminator = 0, eImageFormat img = eImageFormat::PNG, std::size_t size = 4096);
	static std::string GetRoleIcon(const cRole& role, eImageFormat img = eImageFormat::PNG, std::size_t size = 4096);
	static std::string GetRoleIcon(const cSnowflake& role_id, std::string_view hash = {}, eImageFormat img = eImageFormat::PNG, std::size_t size = 4096);
};
#endif /* DISCORD_CDN_H */