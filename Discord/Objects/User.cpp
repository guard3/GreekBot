#include "User.h"
#include "Utils.h"
#include "Discord.h"
cUser::cUser(const json::value& v) : id(v.at("id").as_string().c_str()), username(v.at("username").as_string().c_str()), discriminator(v.at("discriminator").as_string().c_str()) {
	/* Get user avatar image link - https://discord.com/developers/docs/reference#image-formatting-cdn-endpoints */
	const json::value& av = v.at("avatar");
	if (av.is_null())
		avatar = cUtils::Format("%sembed/avatars/%d.png", DISCORD_IMAGE_BASE_URL, strtol(discriminator.c_str(), nullptr, 10) % 5);
	else {
		const char* hash = av.as_string().c_str();
		avatar = cUtils::Format("%savatars/%s/%s.%s?size=4096", DISCORD_IMAGE_BASE_URL, id.ToString(), hash, hash[0] == 'a' && hash[1] == '_' ? "gif" : "png");
	}

	const json::object& o = v.as_object();
	const json::value* p;
	p = o.if_contains("bot");
	bot = p && p->as_bool();
	p = o.if_contains("system");
	system = p && p->as_bool();
}
