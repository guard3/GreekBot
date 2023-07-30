#include "Interaction.h"
#include "json.h"

eInteractionType
tag_invoke(json::value_to_tag<eInteractionType>, const json::value& v) {
	return static_cast<eInteractionType>(json::value_to<int>(v));
}

cApplicationCommandOption::cApplicationCommandOption(const json::value& v, cPtr<const json::value> r):
	m_name(json::value_to<std::string>(v.at("name"))),
	m_type((eApplicationCommandOptionType)v.at("type").as_int64()) {
	switch (m_type) {
		case APP_CMD_OPT_SUB_COMMAND:
		case APP_CMD_OPT_SUB_COMMAND_GROUP: {
			auto& o = m_value.emplace<1>();
			auto& a = v.at("options").as_array();
			o.reserve(a.size());
			for (auto& e : a)
				o.emplace_back(e, r);
			break;
		}
		case APP_CMD_OPT_STRING:
			m_value.emplace<3>(json::value_to<std::string>(v.at("value")));
			break;
		case APP_CMD_OPT_INTEGER:
			m_value.emplace<4>(v.at("value").as_int64());
			break;
		case APP_CMD_OPT_BOOLEAN:
			m_value.emplace<5>(v.at("value").as_bool());
			break;
		case APP_CMD_OPT_USER: {
			json::string_view s = v.at("value").as_string();
			const json::value* a = r->as_object().if_contains("members");
			m_value.emplace<2>(r->at("users").at(s), a ? cHandle::MakeUnique<cMember>(a->at(s)) : uhMember());
			break;
		}
		case APP_CMD_OPT_CHANNEL:
		case APP_CMD_OPT_ROLE:
		case APP_CMD_OPT_MENTIONABLE:
			m_value.emplace<7>(v.at("value"));
			break;
		case APP_CMD_OPT_NUMBER:
			m_value.emplace<6>(v.at("value").as_double());
			break;
		default:
			break;
	}
}

hMember
cApplicationCommandOption::GetMember() {
	try {
		return std::get<1>(std::get<2>(m_value)).get();
	}
	catch (const std::bad_variant_access&) {
		throw xInvalidAttributeError(cUtils::Format("Application command option is not of type %s", a<APP_CMD_OPT_USER>::name));
	}
}

uhMember
cApplicationCommandOption::MoveMember() {
	try {
		return std::move(std::get<1>(std::get<2>(m_value)));
	}
	catch (const std::bad_variant_access&) {
		throw xInvalidAttributeError(cUtils::Format("Application command option is not of type %s", a<APP_CMD_OPT_USER>::name));
	}
}

std::vector<cApplicationCommandOption>&
cApplicationCommandOption::GetOptions() {
	try {
		return std::get<1>(m_value);
	}
	catch (const std::bad_variant_access&) {
		throw xInvalidAttributeError("Application command option is not of type APP_CMD_OPT_SUB_COMMAND or APP_CMD_OPT_SUB_COMMAND_GROUP");
	}
}

cInteractionData<INTERACTION_APPLICATION_COMMAND>::cInteractionData(const json::object& o) : id(o.at("id")), name(o.at("name").as_string().c_str()), type((eApplicationCommandType)o.at("type").as_int64()) {
	if (auto p = o.if_contains("options")) {
		auto& a = p->as_array();
		auto  r = o.if_contains("resolved");
		Options.reserve(a.size());
		for (auto &v: a)
			Options.emplace_back(v, r);
	}
}
cInteractionData<INTERACTION_APPLICATION_COMMAND>::cInteractionData(const json::value& v) : cInteractionData(v.as_object()) {}

cInteractionData<INTERACTION_MESSAGE_COMPONENT>::cInteractionData(const json::object& o) : custom_id(json::value_to<std::string>(o.at("custom_id"))), component_type((eComponentType)o.at("component_type").as_int64()) {
	if (auto p = o.if_contains("values"))
		Values = json::value_to<std::vector<std::string>>(*p);
}
cInteractionData<INTERACTION_MESSAGE_COMPONENT>::cInteractionData(const json::value& v) : cInteractionData(v.as_object()) {}

cInteraction::cInteraction(const json::object &o):
	m_id(json::value_to<cSnowflake>(o.at("id"))),
	m_application_id(json::value_to<cSnowflake>(o.at("application_id"))),
	m_type(json::value_to<eInteractionType>(o.at("type"))),
	m_token(json::value_to<std::string>(o.at("token"))),
	m_version(json::value_to<int>(o.at("version"))) {
	/* Check if interaction was triggered from a guild or DMs */
	if (auto p = o.if_contains("member")) {
		m_um.emplace<cMember>(*p);
		m_guild_id.emplace(o.at("guild_id"));
		m_channel_id.emplace(o.at("channel_id"));
	}
	else m_um.emplace<cUser>(o.at("user"));
	/* Linked message for component interactions */
	if (auto p = o.if_contains("message"))
		m_message.emplace(*p);
	/* Initialize data */
	switch (m_type) {
		case INTERACTION_APPLICATION_COMMAND:
			m_data.emplace<cInteractionData<INTERACTION_APPLICATION_COMMAND>>(o.at("data"));
			break;
		case INTERACTION_MESSAGE_COMPONENT:
			m_data.emplace<cInteractionData<INTERACTION_MESSAGE_COMPONENT>>(o.at("data"));
			break;
		default:
			break;
	}
}

cInteraction::cInteraction(const json::value &v) : cInteraction(v.as_object()) {}