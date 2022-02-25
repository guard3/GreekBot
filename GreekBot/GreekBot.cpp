#include "GreekBot.h"
#include "Database.h"

void
cGreekBot::OnGuildCreate(uhGuild guild) {
	/* Make sure the guild is Learning Greek */
	if (guild->GetId() == m_lmg_id) {
		m_lmg.mutex.lock();
		/* Save guild roles */
		m_lmg.roles = std::move(guild->Roles);
		m_lmg.sorted_roles.clear();
		/* Save guild object */
		m_lmg.guild = std::move(guild);
		m_lmg.mutex.unlock();
	}
}

void
cGreekBot::OnGuildRoleCreate(chSnowflake guild_id, hRole role) {
	if (*guild_id == m_lmg_id) {
		/* A new role was created in Learning Greek */
		m_lmg.mutex.lock();
		m_lmg.roles.push_back(std::move(*role));
		m_lmg.sorted_roles.clear();
		m_lmg.mutex.unlock();
	}
}

void cGreekBot::OnGuildRoleUpdate(chSnowflake guild_id, hRole role) {
	if (*guild_id == m_lmg_id) {
		m_lmg.mutex.lock();
		auto i = std::find_if(m_lmg.roles.begin(), m_lmg.roles.end(), [&role](const cRole& r) { return r.GetId() == role->GetId(); });
		if (i != m_lmg.roles.end())
			*i = std::move(*role);
		m_lmg.sorted_roles.clear();
		m_lmg.mutex.unlock();
	}
}

void
cGreekBot::OnGuildRoleDelete(chSnowflake guild_id, chSnowflake role_id) {
	if (*guild_id == m_lmg_id) {
		m_lmg.mutex.lock();
		auto i = std::find_if(m_lmg.roles.begin(), m_lmg.roles.end(), [&role_id](const cRole& r) { return r.GetId() == *role_id; });
		if (i != m_lmg.roles.end())
			m_lmg.roles.erase(i);
		m_lmg.mutex.unlock();
	}
}

void
cGreekBot::OnInteractionCreate(chInteraction interaction) {
	if (auto data = interaction->GetData<INTERACTION_APPLICATION_COMMAND>()) {
		switch (data->GetCommandId().ToInt()) {
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
		if (*guild_id == m_lmg_id) {
			/* Ignore messages of bots and system users */
			if (msg->GetAuthor().IsBotUser() || msg->GetAuthor().IsSystemUser())
				return;
			/* Update leaderboard */
			cDatabase::UpdateLeaderboard(msg);
		}
	}
}