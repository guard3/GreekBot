#include "Interaction.h"

cInteraction::cInteraction(const json::value &v) : id(v.at("id")), application_id(v.at("application_id")), type(static_cast<eInteractionType>(v.at("type").as_int64())), token(v.at("token").as_string().c_str()), version(static_cast<int>(v.at("version").as_int64())) {
	/* Interpret json value as object */
	auto& o = v.as_object();
	/* Initialize pointers */
	uhSnowflake u_guild_id, u_channel_id;
	uhMember u_member;
	uhUser u_user;
	if (auto c = o.if_contains("guild_id")) u_guild_id = cHandle::MakeUnique<cSnowflake>(*c);
	if (auto c = o.if_contains("channel_id")) u_channel_id = cHandle::MakeUnique<cSnowflake>(*c);
	if (auto c = o.if_contains("member")) u_member = cHandle::MakeUnique<cMember>(*c);
	if (auto c = o.if_contains("user")) u_user = cHandle::MakeUnique<cUser>(*c);
	/* Initialize data */
	switch (type) {
		case INTERACTION_APPLICATION_COMMAND:
			data = new cApplicationCommandInteractionData(v.at("data"));
			break;
		case INTERACTION_MESSAGE_COMPONENT:
			data = new cMessageComponentInteractionData(v.at("data"));
			break;
		default:
			data = nullptr;
			break;
	}
	/* Copy pointers */
	guild_id   = u_guild_id.release();
	channel_id = u_channel_id.release();
	member     = u_member.release();
	user       = u_user.release();
}

cInteraction::cInteraction(const cInteraction& o) : id(o.id), application_id(o.application_id), type(o.type), token(o.token), version(o.version) {
	/* Initialize pointers */
	uhSnowflake u_guild_id   = o.guild_id   ? cHandle::MakeUnique<cSnowflake>(*o.guild_id)   : uhSnowflake();
	uhSnowflake u_channel_id = o.channel_id ? cHandle::MakeUnique<cSnowflake>(*o.channel_id) : uhSnowflake();
	uhMember    u_member     = o.member     ? cHandle::MakeUnique<cMember>(*o.member)        : uhMember();
	uhUser      u_user       = o.user       ? cHandle::MakeUnique<cUser>(*o.user)            : uhUser();
	/* Initialize data */
	switch (type) {
		case INTERACTION_APPLICATION_COMMAND:
			data = new cApplicationCommandInteractionData(*reinterpret_cast<cApplicationCommandInteractionData*>(o.data));
			break;
		case INTERACTION_MESSAGE_COMPONENT:
			data = new cMessageComponentInteractionData(*reinterpret_cast<cMessageComponentInteractionData*>(o.data));
			break;
		default:
			data = nullptr;
			break;
	}
	/* Copy pointers */
	guild_id   = u_guild_id.release();
	channel_id = u_channel_id.release();
	member     = u_member.release();
	user       = u_user.release();
}

cInteraction::cInteraction(cInteraction&& o) noexcept : id(o.id), application_id(o.application_id), type(o.type), token(std::move(o.token)), version(o.version) {
	data       = o.data;
	guild_id   = o.guild_id;
	channel_id = o.channel_id;
	member     = o.member;
	user       = o.user;
	o.data       = nullptr;
	o.guild_id   = nullptr;
	o.channel_id = nullptr;
	o.member     = nullptr;
	o.user       = nullptr;
}

cInteraction::~cInteraction() {
	delete guild_id;
	delete channel_id;
	delete member;
	delete user;
	switch (type) {
		case INTERACTION_APPLICATION_COMMAND:
			delete reinterpret_cast<cApplicationCommandInteractionData*>(data);
			break;
		case INTERACTION_MESSAGE_COMPONENT:
			delete reinterpret_cast<cMessageComponentInteractionData*>(data);
			break;
		default:
			break;
	}
}

cInteraction &cInteraction::operator=(cInteraction o) {
	id = o.id;
	application_id = o.application_id;
	std::swap(type, o.type);
	std::swap(data, o.data);
	std::swap(guild_id, o.guild_id);
	std::swap(channel_id, o.channel_id);
	std::swap(member, o.member);
	std::swap(user, o.user);
	token.swap(o.token);
	version = o.version;
	return *this;
}
