#include "GreekBot.h"
#include "Database.h"

void
cGreekBot::OnGuildCreate(uhGuild guild) {
	/* Make sure the guild is Learning Greek */
	if (guild->GetId()->ToInt() == 350234668680871946) {
		/* Save guild roles */
		m_lmg_roles = std::move(guild->Roles);
		m_lmg_sorted_roles.clear();
		/* Save guild object */
		m_lmg_guild = std::move(guild);
	}
}

void
cGreekBot::OnGuildRoleCreate(chSnowflake guild_id, hRole role) {
	if (m_lmg_guild) {
		if (*m_lmg_guild->GetId() == *guild_id) {
			/* A new role was created in Learning Greek */
			m_lmg_roles.push_back(std::move(*role));
			m_lmg_sorted_roles.clear();
		}
	}
}

void cGreekBot::OnGuildRoleUpdate(chSnowflake guild_id, hRole role) {
	if (m_lmg_guild) {
		if (*m_lmg_guild->GetId() == *guild_id) {
			auto i = std::find_if(m_lmg_roles.begin(), m_lmg_roles.end(), [&role](const cRole& r) { return *r.GetId() == *role->GetId(); });
			if (i != m_lmg_roles.end())
				*i = std::move(*role);
			m_lmg_sorted_roles.clear();
		}
	}
}

void
cGreekBot::OnGuildRoleDelete(chSnowflake guild_id, chSnowflake role_id) {
	if (m_lmg_guild) {
		if (*m_lmg_guild->GetId() == *guild_id) {
			auto i = std::find_if(m_lmg_roles.begin(), m_lmg_roles.end(), [&role_id](const cRole& r) { return *r.GetId() == *role_id; });
			if (i != m_lmg_roles.end())
				m_lmg_roles.erase(i);
		}
	}
}

void
cGreekBot::OnInteractionCreate(chInteraction interaction) {
	if (auto data = interaction->GetData<INTERACTION_APPLICATION_COMMAND>()) {
		switch (data->GetCommandId()->ToInt()) {
			case 878391425568473098:
				/* avatar */
				OnInteraction_avatar(interaction);
				break;
			case 874634186374414356:
				/* role */
				OnInteraction_role(interaction);
				break;
			case 938199801420456066:
				/* rank */
				OnInteraction_rank(interaction);
				break;
			case 938863857466757131:
				/* top */
				OnInteraction_top(interaction);
				break;
			case 904462004071313448:
				/* connect */
				OnInteraction_connect(interaction);
				break;
		}
	}
	else if (auto data = interaction->GetData<INTERACTION_MESSAGE_COMPONENT>()) {
		switch (data->GetComponentType()) {
			case COMPONENT_BUTTON:
				switch (strtol(data->GetCustomId(), nullptr, 10)) {
					case CMP_ID_BUTTON_RANK_HELP:
						OnInteraction_button(interaction);
						break;
				}
				break;
			case COMPONENT_SELECT_MENU:
				OnInteraction_SelectMenu(interaction);
				break;
		}
	}
}

void
cGreekBot::OnMessageCreate(chMessage msg) {
	/* Update leaderboard for Learning Greek */
	if (chSnowflake guild_id = msg->GetGuildId()) {
		if (guild_id->ToInt() == m_lmg_id.ToInt()) {
			/* Ignore messages of bots and system users */
			if (chUser author = msg->GetAuthor(); author->IsBotUser() || author->IsSystemUser())
				return;
			/* Update leaderboard */
			cDatabase::UpdateLeaderboard(msg);
		}
	}
}