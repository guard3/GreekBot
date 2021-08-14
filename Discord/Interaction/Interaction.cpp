#include "Interaction.h"

template<typename T>
static uchHandle<T> init(const json::value& v, const char* str) {
	try {
		return std::make_unique<const T>(v.at(str));
	}
	catch (const std::exception&) {
		return uchHandle<T>();
	}
}

cInteraction::cInteraction(const json::value &v) :
id(v.at("id").as_string().c_str()),
application_id(v.at("application_id").as_string().c_str()),
type(static_cast<eInteractionType>(v.at("type").as_int64())),
guild_id(init<cSnowflake>(v, "guild_id")),
channel_id(init<cSnowflake>(v, "channel_id")),
member(init<cMember>(v, "member")),
user(init<cUser>(v, "user")),
token(v.at("token").as_string().c_str()),
version(static_cast<int>(v.at("version").as_int64())) {
	try {
		auto& d = v.at("data");
		switch (type) {
			case INTERACTION_APPLICATION_COMMAND:
				data = new cApplicationCommandInteractionData(d);
				break;
			case INTERACTION_MESSAGE_COMPONENT:
				data = new cMessageComponentInteractionData(d);
				break;
			default:
				break;
		}
	}
	catch (...) {}
}