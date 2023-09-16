#include "GreekBot.h"
#include "Database.h"
#include "Utils.h"

std::span<const cRole>
cGreekBot::get_lmg_roles() {
	/* Make sure the roles are sorted on position before returning */
	if (!m_bSorted) {
		std::sort(m_lmg_roles.begin(), m_lmg_roles.end(), [](const cRole& a, const cRole& b) {
			return a.GetPosition() > b.GetPosition();
		});
		m_bSorted = true;
	}
	return m_lmg_roles;
}

cColor
cGreekBot::get_lmg_member_color(const cMember& member) {
	/* Find the member's color */
	cColor color;
	for (auto& r : get_lmg_roles()) {
		if (std::find_if(member.GetRoles().begin(), member.GetRoles().end(), [&r](const cSnowflake &id) { return r.GetId() == id; }) != member.GetRoles().end()) {
			if ((color = r.GetColor()))
				break;
		}
	}
	return color;
}

cTask<>
cGreekBot::OnGuildCreate(uhGuild guild) {
	if (guild->GetId() == m_lmg_id) {
		/* If the guild is 'Learning Greek', save its role vector */
		m_lmg_roles = guild->MoveRoles();
		/* Which is most likely not sorted */
		m_bSorted = false;
	}
	m_guilds[guild->GetId()] = std::move(guild);
	co_return;
}

cTask<>
cGreekBot::OnGuildRoleCreate(cSnowflake& guild_id, cRole& role) {
	if (guild_id == m_lmg_id) {
		/* If the guild is 'Learning Greek', save the newly created role */
		m_lmg_roles.push_back(std::move(role));
		/* The vector now is most likely not sorted */
		m_bSorted = false;
	}
	co_return;
}

cTask<>
cGreekBot::OnGuildRoleUpdate(cSnowflake& guild_id, cRole& role) {
	if (guild_id == m_lmg_id) {
		/* If the guild is 'Learning Greek', find the updated role and save the new object */
		auto i = std::find_if(m_lmg_roles.begin(), m_lmg_roles.end(), [&role](const cRole& r) {
			return r.GetId() == role.GetId();
		});
		if (i == m_lmg_roles.end())
			m_lmg_roles.push_back(std::move(role));
		else
			*i = std::move(role);
		/* The new vector is most likely not sorted */
		m_bSorted = false;
	}
	co_return;
}

cTask<>
cGreekBot::OnGuildRoleDelete(cSnowflake& guild_id, cSnowflake& role_id) {
	if (guild_id == m_lmg_id) {
		/* If the guild is 'Learning Greek', just remove all occurrences of the deleted role.
		 * m_bSorted isn't updated because the order doesn't change, so it remains sorted */
		m_lmg_roles.erase(std::remove_if(m_lmg_roles.begin(), m_lmg_roles.end(), [&role_id](const cRole& r) {
			return r.GetId() == role_id;
		}), m_lmg_roles.end());
	}
	co_return;
}

cTask<>
cGreekBot::OnInteractionCreate(const cInteraction& interaction) {
	switch (interaction.GetType()) {
		case INTERACTION_APPLICATION_COMMAND: {
			switch (interaction.GetData<INTERACTION_APPLICATION_COMMAND>().GetCommandId().ToInt()) {
				case 878391425568473098:
					/* avatar */
					co_return co_await OnInteraction_avatar(interaction);
				case 874634186374414356:
					/* role */
					co_return co_await OnInteraction_role(interaction);
				case 938199801420456066:
					/* rank */
					co_return co_await OnInteraction_rank(interaction);
				case 938863857466757131:
					/* top */
					co_return co_await OnInteraction_top(interaction);
				case 1020026874119864381:
					/* prune */
					co_return co_await OnInteraction_prune(interaction);
				case 1031907652541890621:
					/* ban */
					co_return co_await OnInteraction_ban(interaction);
				case 1072131488478404621:
					/* prune (Learning Greek) */
					co_return co_await OnInteraction_prune_lmg(interaction);
				case 904462004071313448:
					/* connect */
					//OnInteraction_connect(&interaction);
				default:
					co_return;
			}
		}
		case INTERACTION_MESSAGE_COMPONENT: {
			switch (auto& data = interaction.GetData<INTERACTION_MESSAGE_COMPONENT>(); data.GetComponentType()) {
				case COMPONENT_BUTTON:
					/* Check custom id */
					if (data.GetCustomId().starts_with("BAN#"))
						co_return co_await OnInteraction_unban(interaction, data.GetCustomId().substr(4));
					if (data.GetCustomId().starts_with("DLT#"))
						co_return co_await OnInteraction_dismiss(interaction, data.GetCustomId().substr(4));
					if (data.GetCustomId().starts_with("NCK#"))
						co_return co_await process_nickname_button(interaction, data.GetCustomId().substr(4));

					switch (cUtils::ParseInt(data.GetCustomId())) {
						case CMP_ID_BUTTON_RANK_HELP:
							co_await OnInteraction_button(interaction);
						default:
							co_return;
					}
				case COMPONENT_SELECT_MENU:
					co_await OnInteraction_SelectMenu(interaction);
				default:
					co_return;
			}
		}
		case INTERACTION_MODAL_SUBMIT:
			co_await process_modal(interaction);
		default:
			co_return;
	}
}

cTask<>
cGreekBot::OnMessageCreate(const cMessage& msg) {
	/* Update leaderboard for Learning Greek */
	if (chSnowflake guild_id = msg.GetGuildId()) {
		if (*guild_id == m_lmg_id) {
			/* Ignore messages of bots and system users */
			if (msg.GetAuthor().IsBotUser() || msg.GetAuthor().IsSystemUser())
				co_return;
			/* Update leaderboard */
			co_await cDatabase::UpdateLeaderboard(msg);
		}
	}
}