#include "User.h"
#include "json.h"

cUser::cUser(const json::object& o):
	id(json::value_to<std::string>(o.at("id"))),
	username(json::value_to<std::string>(o.at("username"))) {
	/* Combine username and discriminator to account for new usernames */
	int disc_int = 0;
	if (auto p = o.if_contains("discriminator")) {
		auto& disc_str = p->as_string();
		disc_int = cUtils::ParseInt(disc_str);
		if (disc_int)
			username = cUtils::Format("%s#%s", username, disc_str);
	}
	/* Get user avatar image link - https://discord.com/developers/docs/reference#image-formatting-cdn-endpoints */
	if (auto s = o.at("avatar").if_string()) {
		const char* hash = s->c_str();
		avatar = cUtils::Format(DISCORD_IMAGE_BASE_URL "avatars/%s/%s.%s?size=4096", id.ToString(), hash, hash[0] == 'a' && hash[1] == '_' ? "gif" : "png");
	}
	else avatar = cUtils::Format(DISCORD_IMAGE_BASE_URL "embed/avatars/%d.png", disc_int ? disc_int % 5 : (id.ToInt() >> 22) % 6);

	const json::value* p;
	p = o.if_contains("bot");
	bot = p && p->as_bool();
	p = o.if_contains("system");
	system = p && p->as_bool();
}

cUser::cUser(const json::value& v) : cUser(v.as_object()) {}