#include "User.h"

cUser::cUser(const json::value& v) : id(v.at("id").as_string().c_str()), username(v.at("username").as_string().c_str()) {
	/* Get discriminator */
	char* tmp;
	discriminator = strtol(v.at("discriminator").as_string().c_str(), &tmp, 10);
	
	/* Get user avatar image link - https://discord.com/developers/docs/reference#image-formatting-cdn-endpoints */
	strcpy(avatar, "https://cdn.discordapp.com/");
	const json::value& av = v.at("avatar");
	if (av.is_null())
		sprintf(avatar + 27, "embed/avatars/%d.png", discriminator % 5);
	else {
		const char* hash = av.as_string().c_str();
		sprintf(avatar + 27, "avatars/%s/%s.%s?size=4096", id.ToString(), hash, hash[0] == 'a' && hash[1] == '_' ? "gif" : "png");
	}
}
