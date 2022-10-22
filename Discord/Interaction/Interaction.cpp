#include "Interaction.h"
#include "json.h"

cInteraction::cInteraction(const json::object &o) : id(o.at("id")), application_id(o.at("application_id")), type((eInteractionType)o.at("type").as_int64()), token(o.at("token").as_string().c_str()), version(o.at("version").as_int64()) {
	/* Check if interaction was triggered from a guild or DMs */
	if (auto p = o.if_contains("member")) {
		member = cHandle::MakeUnique<cMember>(*p);
		guild_id = cHandle::MakeUnique<cSnowflake>(o.at("guild_id"));
		channel_id = cHandle::MakeUnique<cSnowflake>(o.at("channel_id"));
	}
	else user = cHandle::MakeUnique<cUser>(o.at("user"));
	/* Linked message for component interactions */
	if (auto p = o.if_contains("message"))
		message = cHandle::MakeUnique<cMessage>(*p);
	/* Initialize data */
	switch (type) {
		case INTERACTION_APPLICATION_COMMAND:
			data = new cInteractionData<INTERACTION_APPLICATION_COMMAND>(o.at("data"));
			break;
		case INTERACTION_MESSAGE_COMPONENT:
			data = new cInteractionData<INTERACTION_MESSAGE_COMPONENT>(o.at("data"));
			break;
		default:
			data = nullptr;
	}
}

cInteraction::cInteraction(const json::value &v) : cInteraction(v.as_object()) {}

cInteraction::cInteraction(const cInteraction& o) : id(o.id), application_id(o.application_id), type(o.type), token(o.token), version(o.version) {
	/* Copy pointers */
	if (o.user) user = cHandle::MakeUnique<cUser>(*o.user);
	if (o.member) member = cHandle::MakeUnique<cMember>(*o.member);
	if (o.guild_id) guild_id = cHandle::MakeUnique<cSnowflake>(*o.guild_id);
	if (o.channel_id) channel_id = cHandle::MakeUnique<cSnowflake>(*o.channel_id);
	if (o.message) message = cHandle::MakeUnique<cMessage>(*o.message);
	/* Initialize data */
	switch (type) {
		case INTERACTION_APPLICATION_COMMAND:
			data = new cInteractionData<INTERACTION_APPLICATION_COMMAND>(*(hInteractionData<INTERACTION_APPLICATION_COMMAND>)o.data);
			break;
		case INTERACTION_MESSAGE_COMPONENT:
			data = new cInteractionData<INTERACTION_MESSAGE_COMPONENT>(*(hInteractionData<INTERACTION_MESSAGE_COMPONENT>)o.data);
			break;
		default:
			data = nullptr;
			break;
	}
}

cInteraction::cInteraction(cInteraction&& o) noexcept : id(o.id), application_id(o.application_id), type(o.type), data(o.data), version(o.version) {
	std::swap(user,       o.user      );
	std::swap(member,     o.member    );
	std::swap(guild_id,   o.guild_id  );
	std::swap(channel_id, o.channel_id);
	std::swap(message,    o.message   );
	std::swap(token,      o.token     );
	o.data = nullptr;
}

cInteraction::~cInteraction() {
	switch (type) {
		case INTERACTION_APPLICATION_COMMAND:
			delete (hInteractionData<INTERACTION_APPLICATION_COMMAND>)data;
			break;
		case INTERACTION_MESSAGE_COMPONENT:
			delete (hInteractionData<INTERACTION_MESSAGE_COMPONENT>)data;
			break;
		default:
			break;
	}
}

cInteraction &cInteraction::operator=(cInteraction o) {
	std::swap(type,       o.type      );
	std::swap(data,       o.data      );
	std::swap(user,       o.user      );
	std::swap(member,     o.member    );
	std::swap(guild_id,   o.guild_id  );
	std::swap(channel_id, o.channel_id);
	std::swap(token,      o.token     );
	std::swap(message,    o.message   );
	id             = o.id;
	application_id = o.application_id;
	version        = o.version;
	return *this;
}