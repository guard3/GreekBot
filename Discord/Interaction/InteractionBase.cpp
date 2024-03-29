#include "InteractionBase.h"
#include "json.h"
/* ================================================================================================================== */
cInteraction::guild_data::guild_data(const json::value& s, const json::value& v): guild_id(s.as_string()), member(v) {}
/* ================================================================================================================== */
cInteraction::cInteraction(std::uint8_t type, const json::object& o):
	m_ack(false),
	m_type(type),
	m_id(o.at("id").as_string()),
	m_application_id(o.at("application_id").as_string()),
	m_channel_id(o.at("channel_id").as_string()),
	m_token(json::value_to<std::string>(o.at("token"))),
	m_app_permissions(PERM_NONE),
	m_user([](const json::object& o) -> const json::value& {
		const json::value* p;
		return (p = o.if_contains("user")) ? *p : o.at("member").at("user");
	}(o)) {
	/* If the interaction was triggered from DMs... */
	if (o.contains("user"))
		return;
	/* Otherwise collect guild related data */
	m_app_permissions = json::value_to<ePermission>(o.at("app_permissions"));
	m_guild_data.emplace(o.at("guild_id"), o.at("member"));
}