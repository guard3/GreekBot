#include "GreekBot.h"
#include "Database.h"

void
cGreekBot::OnGuildCreate(chGuild guild) {
	if (guild->GetId()->ToInt() == m_lmg_id.ToInt()) {
		cUtils::PrintLog("HELLO LMG!!!");
		for (auto& channel : guild->Channels) {
			if (channel->GetType() == CHANNEL_GUILD_VOICE)
				m_lmg_voice_channels.push_back(channel->GetId()->ToInt());
		}
		for (auto& voice_state : guild->VoiceStates) {
			auto it = std::find(m_lmg_voice_channels.begin(), m_lmg_voice_channels.end(), voice_state->GetChannelId()->ToInt());
			if (it != std::end(m_lmg_voice_channels)) {

			}
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