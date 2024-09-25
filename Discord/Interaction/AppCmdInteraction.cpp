#include "AppCmdInteraction.h"
#include <boost/json.hpp>
/* ================================================================================================================== */
namespace json = boost::json;
/* ================================================================================================================== */
eAppCmdType
tag_invoke(json::value_to_tag<eAppCmdType>, const json::value& v) {
	return static_cast<eAppCmdType>(v.to_number<int>());
}
eAppCmdOptionType
tag_invoke(json::value_to_tag<eAppCmdOptionType>, const json::value& v) {
	return static_cast<eAppCmdOptionType>(v.to_number<int>());
}
cAppCmdOption
tag_invoke(json::value_to_tag<cAppCmdOption>, const json::value& v, cPtr<const json::value> r) {
	return cAppCmdOption{ v, r };
}
/* ========== A simple struct that holds a pair of user and an optional partial member ============================== */
cAppCmdOption::user_data::user_data(const json::value& v, const json::value* p) : user(v) { if (p) member.emplace(*p); }
/* ================================================================================================================== */
cAppCmdOption::cAppCmdOption(const json::value& v, cPtr<const json::value> r):
	m_name(json::value_to<std::string>(v.at("name"))),
	m_type(json::value_to<eAppCmdOptionType>(v.at("type"))) {
	switch (m_type) {
		case APP_CMD_OPT_SUB_COMMAND:
		case APP_CMD_OPT_SUB_COMMAND_GROUP:
			m_value = json::value_to<std::vector<cAppCmdOption>>(v.at("options"), r);
			break;
		case APP_CMD_OPT_STRING:
			m_value = json::value_to<std::string>(v.at("value"));
			break;
		case APP_CMD_OPT_INTEGER:
			m_value = v.at("value").to_number<int>();
			break;
		case APP_CMD_OPT_BOOLEAN:
			m_value = v.at("value").as_bool();
			break;
		case APP_CMD_OPT_USER: {
			json::string_view s = v.at("value").as_string();
			const json::value* pValue = r->as_object().if_contains("members");
			if (pValue) pValue = pValue->as_object().if_contains(s);
			m_value.emplace<user_data>(r->at("users").at(s), pValue);
			break;
		}
		case APP_CMD_OPT_CHANNEL:
		case APP_CMD_OPT_ROLE:
		case APP_CMD_OPT_MENTIONABLE:
			m_value.emplace<cSnowflake>(v.at("value").as_string());
			break;
		default:
			m_value = v.at("value").as_double();
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
/* ========== Option getters for subcommands ======================================================================== */
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
/* ================================================================================================================== */
xAppCmdOptionTypeError::xAppCmdOptionTypeError(eAppCmdOptionType type) : std::invalid_argument([](eAppCmdOptionType type) {
	const char* name;
	switch (type) {
		case APP_CMD_OPT_SUB_COMMAND:
			name = "APP_CMD_OPT_SUB_COMMAND";
			break;
		case APP_CMD_OPT_SUB_COMMAND_GROUP:
			name = "APP_CMD_OPT_SUB_COMMAND_GROUP";
			break;
		case APP_CMD_OPT_STRING:
			name = "APP_CMD_OPT_STRING";
			break;
		case APP_CMD_OPT_INTEGER:
			name = "APP_CMD_OPT_INTEGER";
			break;
		case APP_CMD_OPT_BOOLEAN:
			name = "APP_CMD_OPT_BOOLEAN";
			break;
		case APP_CMD_OPT_USER:
			name = "APP_CMD_OPT_USER";
			break;
		case APP_CMD_OPT_CHANNEL:
			name = "APP_CMD_OPT_CHANNEL";
			break;
		case APP_CMD_OPT_ROLE:
			name = "APP_CMD_OPT_ROLE";
			break;
		case APP_CMD_OPT_MENTIONABLE:
			name = "APP_CMD_OPT_MENTIONABLE";
			break;
		default:
			name = "APP_CMD_OPT_NUMBER";
			break;
	}
	return std::format("Option is not of type {}", name);
} (type)) {}
/* ========== Value getters for all option types ==================================================================== */
template<> std::string_view
cAppCmdOption::GetValue<APP_CMD_OPT_STRING>() const {
	if (auto pValue = std::get_if<std::string>(&m_value))
		return *pValue;
	throw xAppCmdOptionTypeError(APP_CMD_OPT_STRING);
}
template<> int
cAppCmdOption::GetValue<APP_CMD_OPT_INTEGER>() const {
	if (auto pValue = std::get_if<int>(&m_value))
		return *pValue;
	throw xAppCmdOptionTypeError(APP_CMD_OPT_INTEGER);
}
template<> bool
cAppCmdOption::GetValue<APP_CMD_OPT_BOOLEAN>() const {
	if (auto pValue = std::get_if<bool>(&m_value))
		return *pValue;
	throw xAppCmdOptionTypeError(APP_CMD_OPT_BOOLEAN);
}
template<> double
cAppCmdOption::GetValue<APP_CMD_OPT_NUMBER>() const {
	if (auto pValue = std::get_if<double>(&m_value))
		return *pValue;
	throw xAppCmdOptionTypeError(APP_CMD_OPT_NUMBER);
}
template<> const cSnowflake&
cAppCmdOption::GetValue<APP_CMD_OPT_CHANNEL>() const {
	if (auto pValue = std::get_if<cSnowflake>(&m_value); pValue && m_type == APP_CMD_OPT_CHANNEL)
		return *pValue;
	throw xAppCmdOptionTypeError(APP_CMD_OPT_CHANNEL);
}
template<> const cSnowflake&
cAppCmdOption::GetValue<APP_CMD_OPT_ROLE>() const {
	if (auto pValue = std::get_if<cSnowflake>(&m_value); pValue && m_type == APP_CMD_OPT_ROLE)
		return *pValue;
	throw xAppCmdOptionTypeError(APP_CMD_OPT_ROLE);
}
template<> const cSnowflake&
cAppCmdOption::GetValue<APP_CMD_OPT_MENTIONABLE>() const {
	if (auto pValue = std::get_if<cSnowflake>(&m_value); pValue && m_type == APP_CMD_OPT_MENTIONABLE)
		return *pValue;
	throw xAppCmdOptionTypeError(APP_CMD_OPT_MENTIONABLE);
}
template<> std::pair<chUser, chPartialMember>
cAppCmdOption::GetValue<APP_CMD_OPT_USER>() const {
	if (auto pValue = std::get_if<user_data>(&m_value))
		return { &pValue->user, pValue->member ? &*pValue->member : nullptr };
	throw xAppCmdOptionTypeError(APP_CMD_OPT_USER);
}
/* ========== Helper value mover ==================================================================================== */
std::string
cAppCmdOption::move_value_string() {
	if (auto pValue = std::get_if<std::string>(&m_value))
		return std::move(*pValue);
	throw xAppCmdOptionTypeError(APP_CMD_OPT_STRING);
}
std::pair<cUser, std::optional<cPartialMember>>
cAppCmdOption::move_value_user() {
	if (auto pValue = std::get_if<user_data>(&m_value))
		return { std::move(pValue->user), std::move(pValue->member) };
	throw xAppCmdOptionTypeError(APP_CMD_OPT_USER);
}
/* ================================================================================================================== */
using namespace detail; // Expose interaction types
cAppCmdInteraction::cAppCmdInteraction(const json::value& v): cAppCmdInteraction(v.as_object()) {}
cAppCmdInteraction::cAppCmdInteraction(const json::object& o): cAppCmdInteraction(o, o.at("data").as_object()) {}
cAppCmdInteraction::cAppCmdInteraction(const json::object& o, const json::object& d):
	cInteraction(INTERACTION_APPLICATION_COMMAND, o),
	m_id(d.at("id").as_string()),
	m_type(json::value_to<eAppCmdType>(d.at("type"))),
	m_name(json::value_to<std::string>(d.at("name"))) {
	/* Parse guild_id */
	if (auto p = d.if_contains("guild_id"))
		m_guild_id.emplace(p->as_string());
	/* Parse options */
	switch (m_type) {
		case APP_CMD_CHAT_INPUT:
			if (auto p = d.if_contains("options"))
				m_options = json::value_to<std::vector<cAppCmdOption>>(*p, d.if_contains("resolved"));
			break;
		case APP_CMD_USER: {
			m_options.emplace_back(APP_CMD_USER, d.at("target_id").as_string(), d.at("resolved").as_object());
			break;
		}
		//case APP_CMD_MESSAGE:
		default:
			break;
	}
}