#include "CDN.h"

static constexpr auto base_url = "https://cdn.discordapp.com/";

static const char* get_extension(eImageFormat img) noexcept {
	/* The file types */
	static const char types[4][5]{ "png", "jpg", "jpeg", "webp" };
	/* Return appropriate string with bounds checking */
	auto i = static_cast<unsigned>(img);
	if (i >= std::size(types))
		i = 0;
	return types[i];
}

std::string
cCDN::GetDefaultUserAvatar(const cSnowflake& user_id, std::uint16_t discr) {
	return fmt::format("{}embed/avatars/{}.png", base_url, discr ? discr % 5 : (user_id.ToInt() >> 22) % 6);
}
std::string
cCDN::GetDefaultUserAvatar(const cUser& user) {
	return GetDefaultUserAvatar(user.GetId(), user.GetDiscriminator());
}
std::string
cCDN::GetUserAvatar(const cUser& user, eImageFormat img, size_t size) {
	return GetUserAvatar(user.GetId(), user.GetAvatar(), user.GetDiscriminator(), img, size);
}
std::string
cCDN::GetUserAvatar(const cSnowflake& user_id, std::string_view hash, std::uint16_t discr, eImageFormat img, size_t size) {
	if (hash.empty())
		return GetDefaultUserAvatar(user_id, discr);
	return fmt::format("{}avatars/{}/{}.{}?size={}", base_url, user_id, hash, hash.starts_with("a_") ? "gif" : get_extension(img), size);
}
