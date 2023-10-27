#include "User.h"
#include "Utils.h"
#include "json.h"
#include <fmt/format.h>

cUser::cUser(const json::object& o):
	id(json::value_to<cSnowflake>(o.at("id"))),
	username(json::value_to<std::string>(o.at("username"))) {
	/* Combine username and discriminator to account for new usernames */
	int disc_int = 0;
	if (auto p = o.if_contains("discriminator")) {
		auto disc_str = json::value_to<std::string_view>(*p);
		if ((disc_int = cUtils::ParseInt(disc_str)))
			username = fmt::format("{}#{}", username, disc_str);
	}
	/* Get user avatar image link - https://discord.com/developers/docs/reference#image-formatting-cdn-endpoints */
	if (auto s = o.at("avatar").if_string()) {
		auto hash = static_cast<std::string_view>(*s);
		avatar = fmt::format("{}avatars/{}/{}.{}?size=4096", DISCORD_IMAGE_BASE_URL, id, hash, hash.starts_with("a_") ? "gif" : "png");
	} else {
		avatar = fmt::format("{}embed/avatars/{}.png", DISCORD_IMAGE_BASE_URL, disc_int ? disc_int % 5 : (id.ToInt() >> 22) % 6);
	}
	const json::value* p;
	p = o.if_contains("bot");
	bot = p && p->as_bool();
	p = o.if_contains("system");
	system = p && p->as_bool();
}

cUser::cUser(const json::value& v) : cUser(v.as_object()) {}