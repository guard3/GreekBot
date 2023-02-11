#include "Interaction.h"
#include "json.h"

cInteraction::cInteraction(const json::object &o):
	m_id(o.at("id")),
	m_application_id(o.at("application_id")),
	m_type((eInteractionType)o.at("type").as_int64()),
	m_token(json::value_to<std::string>(o.at("token"))),
	m_version(o.at("version").as_int64()) {
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