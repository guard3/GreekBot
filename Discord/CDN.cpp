#include "CDN.h"

static constexpr auto base_url = "https://cdn.discordapp.com/";

std::string
cCDN::GetDefaultUserAvatar(crefUser user) {
	uint16_t d = user.GetDiscriminator();
	return fmt::format("{}embed/avatars/{}.png", base_url, d ? d % 5 : (user.GetId().ToInt() >> 22) % 6);
}

std::string
cCDN::GetUserAvatar(crefUser user) {
	if (auto hash = user.GetAvatar(); !hash.empty())
		return fmt::format("{}avatars/{}/{}.{}?size=4096", base_url, user.GetId(), hash, hash.starts_with("a_") ? "gif" : "png");
	return GetDefaultUserAvatar(user);
}
