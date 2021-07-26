#include "User.h"

cUser::cUser(const json::value& v) : id(v.at("id").as_string()), username(v.at("username").as_string()), discriminator(v.at("discriminator").as_string()) {
	strcpy(avatar, "https://cdn.discordapp.com/");
	const json::value& av = v.at("avatar");
	if (av.is_null()) {
		int discriminator_int;
		sscanf(discriminator.c_str(), "%d", &discriminator_int);
		sprintf(avatar + 27, "embed/avatars/%d.png", discriminator_int % 5);
	}
	else {
		const char* hash = av.as_string().c_str();
		sprintf(avatar + 27, "avatars/%s/%s.%s?size=4096", id.c_str(), hash, hash[0] == 'a' && hash[1] == '_' ? "gif" : "png");
	}
}
