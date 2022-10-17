#include "Interaction.h"

cApplicationCommandOption::cApplicationCommandOption(const json::value& v, const json::value* r) : m_name(json::value_to<std::string>(v.at("name"))), m_type((eApplicationCommandOptionType)v.at("type").as_int64()) {
	switch (m_type) {
		case APP_CMD_OPT_SUB_COMMAND:
		case APP_CMD_OPT_SUB_COMMAND_GROUP: {
			auto& a = v.at("options").as_array();
			std::vector<cApplicationCommandOption> t;
			t.reserve(a.size());
			for (auto& e : a)
				t.emplace_back(e, r);
			new(&m_options) std::vector<cApplicationCommandOption>(std::move(t));
			break;
		}
		case APP_CMD_OPT_STRING:
			new(&m_value_string) std::string(json::value_to<std::string>(v.at("value")));
			break;
		case APP_CMD_OPT_INTEGER:
			m_value_integer = v.at("value").as_int64();
			break;
		case APP_CMD_OPT_BOOLEAN:
			m_value_boolean = v.at("value").as_bool();
			break;
		case APP_CMD_OPT_USER: {
			if (!r)
				throw std::runtime_error("aaa");
			json::string_view s = v.at("value").as_string();
			const json::value* a = r->as_object().if_contains("members");
			new(&m_value_user) std::tuple<cUser, cWrapper<cMember>>(r->at("users").at(s), a ? a->at(s) : cWrapper<cMember>());
			break;
		}
		case APP_CMD_OPT_CHANNEL:
		case APP_CMD_OPT_ROLE:
		case APP_CMD_OPT_MENTIONABLE:
			new(&m_value_snowflake) cSnowflake(v.at("value"));
			break;
		case APP_CMD_OPT_NUMBER:
			m_value_number = v.at("value").as_double();
			break;
		default:
			break;
	}
}

cApplicationCommandOption::cApplicationCommandOption(const cApplicationCommandOption& o) : m_name(o.m_name), m_type(o.m_type) {
	switch (m_type) {
		case APP_CMD_OPT_SUB_COMMAND:
		case APP_CMD_OPT_SUB_COMMAND_GROUP:
			new(&m_options) std::vector<cApplicationCommandOption>(o.m_options);
			break;
		case APP_CMD_OPT_STRING:
			new(&m_value_string) std::string(o.m_value_string);
			break;
		case APP_CMD_OPT_INTEGER:
			m_value_integer = o.m_value_integer;
			break;
		case APP_CMD_OPT_BOOLEAN:
			m_value_boolean = o.m_value_boolean;
			break;
		case APP_CMD_OPT_USER:
			new(&m_value_user) std::tuple<cUser, cWrapper<cMember>>(o.m_value_user);
			break;
		case APP_CMD_OPT_CHANNEL:
		case APP_CMD_OPT_ROLE:
		case APP_CMD_OPT_MENTIONABLE:
			new(&m_value_snowflake) cSnowflake(o.m_value_snowflake);
			break;
		case APP_CMD_OPT_NUMBER:
			m_value_number = o.m_value_number;
		default:
			break;
	}
}

cApplicationCommandOption::cApplicationCommandOption(cApplicationCommandOption&& o) noexcept : m_name(std::move(o.m_name)), m_type(o.m_type) {
	switch (m_type) {
		case APP_CMD_OPT_SUB_COMMAND:
		case APP_CMD_OPT_SUB_COMMAND_GROUP:
			new(&m_options) std::vector<cApplicationCommandOption>(std::move(o.m_options));
			break;
		case APP_CMD_OPT_STRING:
			new(&m_value_string) std::string(std::move(o.m_value_string));
			break;
		case APP_CMD_OPT_INTEGER:
			m_value_integer = o.m_value_integer;
			break;
		case APP_CMD_OPT_BOOLEAN:
			m_value_boolean = o.m_value_boolean;
			break;
		case APP_CMD_OPT_USER:
			new(&m_value_user) std::tuple<cUser, cWrapper<cMember>>(std::move(o.m_value_user));
			break;
		case APP_CMD_OPT_CHANNEL:
		case APP_CMD_OPT_ROLE:
		case APP_CMD_OPT_MENTIONABLE:
			new(&m_value_snowflake) cSnowflake(std::move(o.m_value_snowflake));
			break;
		case APP_CMD_OPT_NUMBER:
			m_value_number = o.m_value_number;
			break;
		default:
			break;
	}
}

cApplicationCommandOption::~cApplicationCommandOption() {
	switch (m_type) {
		case APP_CMD_OPT_SUB_COMMAND:
		case APP_CMD_OPT_SUB_COMMAND_GROUP:
			m_options.~vector();
		case APP_CMD_OPT_STRING:
			m_value_string.~basic_string();
			break;
		case APP_CMD_OPT_USER:
			m_value_user.~tuple();
			break;
		case APP_CMD_OPT_CHANNEL:
		case APP_CMD_OPT_ROLE:
		case APP_CMD_OPT_MENTIONABLE:
			m_value_snowflake.~cSnowflake();
			break;
		default:
			break;
	}
}

cApplicationCommandOption& cApplicationCommandOption::operator=(cApplicationCommandOption o) {
	/* If the types don't match, explicitly destroy and reinitialize */
	if (m_type != o.m_type) {
		this->~cApplicationCommandOption();
		new(this) cApplicationCommandOption(std::move(o));
		return *this;
	}
	/* If types match, safely swap members */
	m_type = o.m_type;
	m_name.swap(o.m_name);
	switch (m_type) {
		case APP_CMD_OPT_SUB_COMMAND:
		case APP_CMD_OPT_SUB_COMMAND_GROUP:
			m_options.swap(o.m_options);
			break;
		case APP_CMD_OPT_STRING:
			m_value_string.swap(o.m_value_string);
			break;
		case APP_CMD_OPT_INTEGER:
			m_value_integer = o.m_value_integer;
			break;
		case APP_CMD_OPT_BOOLEAN:
			m_value_boolean = o.m_value_boolean;
			break;
		case APP_CMD_OPT_USER:
			m_value_user.swap(o.m_value_user);
			break;
		case APP_CMD_OPT_CHANNEL:
		case APP_CMD_OPT_ROLE:
		case APP_CMD_OPT_MENTIONABLE:
			m_value_snowflake = o.m_value_snowflake;
			break;
		case APP_CMD_OPT_NUMBER:
			m_value_number = o.m_value_number;
		default:
			break;
	}
	return *this;
}

hMember
cApplicationCommandOption::GetMember() {
	if (m_type == APP_CMD_OPT_USER)
		return std::get<1>(m_value_user).Get();
	throw xInvalidAttributeError(cUtils::Format("Application command option is not of type %s", a<APP_CMD_OPT_USER>::name));
}

uhMember
cApplicationCommandOption::MoveMember() {
	if (m_type == APP_CMD_OPT_USER)
		return std::get<1>(m_value_user).Move();
	throw xInvalidAttributeError(cUtils::Format("Application command option is not of type %s", a<APP_CMD_OPT_USER>::name));
}

std::vector<cApplicationCommandOption>&
cApplicationCommandOption::GetOptions() {
	if (m_type == APP_CMD_OPT_SUB_COMMAND || m_type == APP_CMD_OPT_SUB_COMMAND_GROUP)
		return m_options;
	throw xInvalidAttributeError("Application command option is not of type APP_CMD_OPT_SUB_COMMAND or APP_CMD_OPT_SUB_COMMAND_GROUP");
}