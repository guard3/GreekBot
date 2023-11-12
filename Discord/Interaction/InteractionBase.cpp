#include "Interaction.h"
#include "json.h"

eInteractionType
tag_invoke(json::value_to_tag<eInteractionType>, const json::value& v) {
	return static_cast<eInteractionType>(v.to_number<int>());
}
eAppCmdType
tag_invoke(json::value_to_tag<eAppCmdType>, const json::value& v) {
	return static_cast<eAppCmdType>(v.to_number<int>());
}
eAppCmdOptionType
tag_invoke(json::value_to_tag<eAppCmdOptionType>, const json::value& v) {
	return static_cast<eAppCmdOptionType>(v.to_number<int>());
}

cAppCmdOption::user_data::user_data(const json::value& v, const json::value* p) : user(v) {
	if (p)
		member.emplace(*p);
}

cAppCmdOption::cAppCmdOption(const json::value& v, cPtr<const json::value> r):
	m_name(json::value_to<std::string>(v.at("name"))),
	m_type(json::value_to<eAppCmdOptionType>(v.at("type"))) {
	switch (m_type) {
		case APP_CMD_OPT_SUB_COMMAND:
		case APP_CMD_OPT_SUB_COMMAND_GROUP: {
			auto& o = m_value.emplace<options_t>();
			auto& a = v.at("options").as_array();
			o.reserve(a.size());
			for (auto& e : a)
				o.emplace_back(e, r);
			break;
		}
		case APP_CMD_OPT_STRING:
			m_value.emplace<std::string>(json::value_to<std::string>(v.at("value")));
			break;
		case APP_CMD_OPT_INTEGER:
			m_value.emplace<int>(v.at("value").to_number<int>());
			break;
		case APP_CMD_OPT_BOOLEAN:
			m_value.emplace<bool>(v.at("value").as_bool());
			break;
		case APP_CMD_OPT_USER: {
			json::string_view s = v.at("value").as_string();
			const json::value* a = r->as_object().if_contains("members");
			m_value.emplace<user_data>(r->at("users").at(s), a ? a->as_object().if_contains(s) : nullptr);
			break;
		}
		case APP_CMD_OPT_CHANNEL:
		case APP_CMD_OPT_ROLE:
		case APP_CMD_OPT_MENTIONABLE:
			m_value.emplace<cSnowflake>(json::value_to<std::string_view>(v.at("value")));
			break;
		case APP_CMD_OPT_NUMBER:
			m_value.emplace<double>(v.at("value").as_double());
			break;
		default:
			break;
	}
}

cAppCmdOption::cAppCmdOption(eAppCmdType type, std::string_view id, const json::object& resolved) {
	if (type == APP_CMD_USER) {
		m_type = APP_CMD_OPT_USER;
		const json::value* a = resolved.if_contains("members");
		m_value.emplace<user_data>(resolved.at("users").at(id), a ? a->as_object().if_contains(id) : nullptr);
	}
}

std::span<const cAppCmdOption>
cAppCmdOption::GetOptions() const noexcept {
	if (auto p = std::get_if<std::vector<cAppCmdOption>>(&m_value))
		return *p;
	return {};
}
std::span<cAppCmdOption>
cAppCmdOption::GetOptions() noexcept {
	if (auto p = std::get_if<std::vector<cAppCmdOption>>(&m_value))
		return *p;
	return {};
}
std::vector<cAppCmdOption>
cAppCmdOption::MoveOptions() noexcept {
	if (auto p = std::get_if<std::vector<cAppCmdOption>>(&m_value))
		return std::move(*p);
	return {};
}

cInteraction::guild_data::guild_data(const json::value& s, const json::value& v): guild_id(json::value_to<cSnowflake>(s)), member(v) {}

cInteraction::cInteraction(eInteractionType type, const json::object& o):
	m_type(type),
	m_id(json::value_to<std::string_view>(o.at("id"))),
	m_application_id(json::value_to<std::string_view>(o.at("application_id"))),
	m_token(json::value_to<std::string>(o.at("token"))),
	m_app_permissions(PERM_NONE),
	m_user([](const json::object& o) -> const json::value& {
		const json::value* p;
		return (p = o.if_contains("user")) ? *p : o.at("member").at("user");
	}(o)) {
	const json::value* p;
	if ((p = o.if_contains("channel_id")))
		m_channel_id.emplace(json::value_to<std::string_view>(*p));
	/* If the interaction was triggered from DMs... */
	if (o.contains("user"))
		return;
	/* Otherwise collect guild related data */
	m_app_permissions = json::value_to<ePermission>(o.at("app_permissions"));
	m_guild_data.emplace(o.at("guild_id"), o.at("member"));
}