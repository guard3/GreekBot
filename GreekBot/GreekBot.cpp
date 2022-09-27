#include "GreekBot.h"
#include "Database.h"

cColor
cGreekBot::get_lmg_member_color(const cMember& member) {
	/* Make sure roles are sorted based on position */
	if (m_lmg.sorted_roles.empty()) {
		for (auto &r: m_lmg.roles)
			m_lmg.sorted_roles.push_back(&r);
		std::sort(m_lmg.sorted_roles.begin(), m_lmg.sorted_roles.end(), [](chRole a, chRole b) { return a->GetPosition() > b->GetPosition(); });
	}
	/* Find the member's color */
	cColor color;
	for (auto& r : m_lmg.sorted_roles) {
		if (std::find_if(member.Roles.begin(), member.Roles.end(), [&r](const cSnowflake &id) { return r->GetId() == id; }) != member.Roles.end()) {
			if ((color = r->GetColor()))
				break;
		}
	}
	return color;
}

cTask<>
cGreekBot::OnGuildCreate(uhGuild guild) {
	/* Make sure the guild is Learning Greek */
	if (guild->GetId() == m_lmg_id) {
		/* Save guild roles */
		m_lmg.roles = std::move(guild->Roles);
		m_lmg.sorted_roles.clear();
		/* Save guild object */
		m_lmg.guild = cHandle::MakeUnique<cGuild>(*guild);
	}
	m_guilds[guild->GetId()] = std::move(guild);
	co_return;
}

cTask<>
cGreekBot::OnGuildRoleCreate(cSnowflake& guild_id, cRole& role) {
	if (guild_id == m_lmg_id) {
		/* A new role was created in Learning Greek */
		m_lmg.roles.push_back(std::move(role));
		m_lmg.sorted_roles.clear();
	}
	co_return;
}

cTask<>
cGreekBot::OnGuildRoleUpdate(cSnowflake& guild_id, cRole& role) {
	if (guild_id == m_lmg_id) {
		auto i = std::find_if(m_lmg.roles.begin(), m_lmg.roles.end(), [&role](const cRole& r) { return r.GetId() == role.GetId(); });
		if (i != m_lmg.roles.end())
			*i = std::move(role);
		m_lmg.sorted_roles.clear();
	}
	co_return;
}

cTask<>
cGreekBot::OnGuildRoleDelete(cSnowflake& guild_id, cSnowflake& role_id) {
	if (guild_id == m_lmg_id) {
		auto i = std::find_if(m_lmg.roles.begin(), m_lmg.roles.end(), [&role_id](const cRole& r) { return r.GetId() == role_id; });
		if (i != m_lmg.roles.end())
			m_lmg.roles.erase(i);
		m_lmg.sorted_roles.clear();
	}
	co_return;
}

cTask<>
cGreekBot::OnInteractionCreate(const cInteraction& interaction) {
	if (auto data = interaction.GetData<INTERACTION_APPLICATION_COMMAND>()) {
		switch (data->GetCommandId().ToInt()) {
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
			case 1022285237260140674:
				/* ban */
				co_return co_await OnInteraction_ban(interaction);
			case 904462004071313448:
				/* connect */
				//OnInteraction_connect(&interaction);
				break;
		}
	}
	else if (auto data = interaction.GetData<INTERACTION_MESSAGE_COMPONENT>()) {
		switch (data->GetComponentType()) {
			case COMPONENT_BUTTON:
				switch (strtol(data->GetCustomId(), nullptr, 10)) {
					case CMP_ID_BUTTON_RANK_HELP:
						co_await OnInteraction_button(interaction);
						break;
				}
				break;
			case COMPONENT_SELECT_MENU:
				co_await OnInteraction_SelectMenu(interaction);
				break;
		}
	}
	co_return;
}

cTask<>
cGreekBot::OnMessageCreate(cMessage& msg) {
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