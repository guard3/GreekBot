#include "Interaction.h"

cInteractionDataOption::cInteractionDataOption(const json::value& v, const json::value* r) :
name(v.at("name").as_string().c_str()),
type(static_cast<eApplicationCommandType>(v.at("type").as_int64())),
value([](const json::value& v) {
	try {
		return v.at("value");
	}
	catch (...) {
		return json::value();
	}
} (v)) {
	if (!r) return;
	try {
		switch (type) {
			case APP_COMMAND_USER: {
				const char* user_id = value.as_string().c_str();
				m_user = std::make_unique<cUser>(r->at("users").at(user_id));
				m_member = std::make_unique<cMember>(r->at("members").at(user_id));
				break;
			}
			case APP_COMMAND_CHANNEL:
				m_snowflake = std::make_unique<cSnowflake>(r->at("channels").at(value.as_string().c_str()));
				break;
			case APP_COMMAND_ROLE:
				m_snowflake = std::make_unique<cSnowflake>(r->at("roles").at(value.as_string().c_str()));
			default:
				break;
		}
	}
	catch (...) {}
}

cInteractionData::cInteractionData(const json::value &v) : id(v.at("id")), name(v.at("name").as_string().c_str()) {
	if (auto o = v.if_object()) {
		const json::value* r = o->if_contains("resolved");
		auto& opt = const_cast<std::vector<chInteractionDataOption>&>(Options);
		try {
			const json::array& a = v.at("options").as_array();
			options.reserve(a.size());
			opt.reserve(a.size());
			for (auto& value : a) {
				options.emplace_back(value, r);
				opt.push_back(&options.back());
			}
		}
		catch (...) {
			options.clear();
			opt.clear();
		}
	}
}

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
data(v.at("data")),
guild_id(init<cSnowflake>(v, "guild_id")),
channel_id(init<cSnowflake>(v, "channel_id")),
member(init<cMember>(v, "member")),
user(init<cUser>(v, "user")),
token(v.at("token").as_string().c_str()),
version(static_cast<int>(v.at("version").as_int64())) {}