#include "User.h"
#include "json.h"

cUser::cUser(const json::object& o) : id(o.at("id").as_string().c_str()), username(o.at("username").as_string().c_str()), discriminator(o.at("discriminator").as_string().c_str()) {
	/* Get user avatar image link - https://discord.com/developers/docs/reference#image-formatting-cdn-endpoints */
	if (auto s = o.at("avatar").if_string()) {
		const char* hash = s->c_str();
		avatar = cUtils::Format("%savatars/%s/%s.%s?size=4096", DISCORD_IMAGE_BASE_URL, id.ToString(), hash, hash[0] == 'a' && hash[1] == '_' ? "gif" : "png");
	}
	else avatar = cUtils::Format("%sembed/avatars/%d.png", DISCORD_IMAGE_BASE_URL, strtol(discriminator.c_str(), nullptr, 10) % 5);

	const json::value* p;
	p = o.if_contains("bot");
	bot = p && p->as_bool();
	p = o.if_contains("system");
	system = p && p->as_bool();
}

cUser::cUser(const json::value& v) : cUser(v.as_object()) {}