#include "Interaction.h"
#include "json.h"

eInteractionType
tag_invoke(json::value_to_tag<eInteractionType>, const json::value& v) {
	return static_cast<eInteractionType>(v.to_number<int>());
}
eApplicationCommandType
tag_invoke(json::value_to_tag<eApplicationCommandType>, const json::value& v) {
	return static_cast<eApplicationCommandType>(v.to_number<int>());
}
eApplicationCommandOptionType
tag_invoke(json::value_to_tag<eApplicationCommandOptionType>, const json::value& v) {
	return static_cast<eApplicationCommandOptionType>(v.to_number<int>());
}

cApplicationCommandOption::cApplicationCommandOption(const json::value& v, cPtr<const json::value> r):
	m_name(json::value_to<std::string>(v.at("name"))),
	m_type(json::value_to<eApplicationCommandOptionType>(v.at("type"))) {
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
			m_value.emplace<7>(json::value_to<cSnowflake>(v.at("value")));
			break;
		case APP_CMD_OPT_NUMBER:
			m_value.emplace<6>(v.at("value").as_double());
			break;
		default:
			break;
	}
}

cApplicationCommandOption::cApplicationCommandOption(eApplicationCommandType type, std::string_view id, const json::object& resolved) {
	if (type == APP_CMD_USER) {
		m_type = APP_CMD_OPT_USER;
		const json::value* a = resolved.if_contains("members");
		m_value.emplace<2>(resolved.at("users").at(id), a ? cHandle::MakeUnique<cMember>(a->at(id)) : uhMember());
	}
}

hMember
cApplicationCommandOption::GetMember() {
	try {
		return std::get<1>(std::get<2>(m_value)).get();
	}
	catch (const std::bad_variant_access&) {
		throw xInvalidAttributeError(fmt::format("Application command option is not of type {}", a<APP_CMD_OPT_USER>::name));
	}
}

uhMember
cApplicationCommandOption::MoveMember() {
	try {
		return std::move(std::get<1>(std::get<2>(m_value)));
	}
	catch (const std::bad_variant_access&) {
		throw xInvalidAttributeError(fmt::format("Application command option is not of type {}", a<APP_CMD_OPT_USER>::name));
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

cModalSubmitData::cModalSubmitData(const json::value& v):
	m_custom_id(json::value_to<std::string>(v.at("custom_id"))),
	m_value(json::value_to<std::string>(v.at("value"))) {}

cInteraction::guild_data::guild_data(std::string_view s, const json::value& v): guild_id(s), member(v) {}

cInteraction::cInteraction(eInteractionType type, const json::value& v): cInteraction(type, v.as_object()) {}
cInteraction::cInteraction(eInteractionType type, const json::object& o):
	m_type(type),
	m_id(json::value_to<std::string_view>(o.at("id"))),
	m_application_id(json::value_to<std::string_view>(o.at("application_id"))),
	m_token(json::value_to<std::string>(o.at("token"))),
	m_app_permissions(PERM_NONE) {
	const json::value* p;
	if ((p = o.if_contains("channel_id")))
		m_channel_id.emplace(json::value_to<std::string_view>(*p));
	/* If the interaction was triggered from DMs... */
	if ((p = o.if_contains("user"))) {
		m_variant.emplace<cUser>(*p);
		return;
	}
	/* Otherwise collect guild related data */
	m_app_permissions = json::value_to<ePermission>(o.at("app_permissions"));
	m_variant.emplace<guild_data>(json::value_to<std::string_view>(o.at("guild_id")), o.at("member"));
}