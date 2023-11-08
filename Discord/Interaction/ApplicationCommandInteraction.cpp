#include "ApplicationCommandInteraction.h"
#include "json.h"

cApplicationCommandInteraction::cApplicationCommandInteraction(const json::value& v): cApplicationCommandInteraction(v.as_object()) {}
cApplicationCommandInteraction::cApplicationCommandInteraction(const json::object& o): cApplicationCommandInteraction(o, o.at("data").as_object()) {}
cApplicationCommandInteraction::cApplicationCommandInteraction(const json::object& o, const json::object& d):
	cInteraction(INTERACTION_APPLICATION_COMMAND, o),
	m_id(json::value_to<std::string_view>(d.at("id"))),
	m_type(json::value_to<eApplicationCommandType>(d.at("type"))),
	m_name(json::value_to<std::string>(d.at("name"))) {
	/* Parse guild_id */
	const json::value* p;
	if ((p = d.if_contains("guild_id")))
		m_guild_id.emplace(json::value_to<std::string_view>(*p));

	switch (m_type) {
		case APP_CMD_CHAT_INPUT:
			if ((p = d.if_contains("options"))) {
				auto& a = p->as_array();
				auto  r = d.if_contains("resolved");
				m_options.reserve(a.size());
				for (auto& opt : a)
					m_options.emplace_back(opt, r);
			}
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