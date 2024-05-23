#include "GreekBot.h"
#include "Database.h"
#include "Utils.h"

[[noreturn]]
void unhandled_exception(const char* name) {
	throw unhandled_exception_t{ name, std::current_exception() };
}

enum : uint32_t {
	CMP_ID_LEADERBOARD_HELP = 0x4ECEBDEC,
	CMP_ID_STARBOARD_HELP   = 0x33330ADE,
	CMP_ID_PROFICIENCY_MENU = 0xAE90F56B,
	CMP_ID_BOOSTER_MENU     = 0x90DD7D88,

	MODAL_NICKNAME = 0x41732EB9,
	MODAL_BAN      = 0x9839CB2A
};

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
cGreekBot::get_lmg_member_color(const cPartialMember& member) {
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
cGreekBot::OnInteractionCreate(cInteraction& i) {
	try {
		/* Delegate interaction to the appropriate handler */
		co_return co_await i.Visit([this](auto& i) { return process_interaction(i); });
	} catch (const unhandled_exception_t& u) {
		/* If an exception escaped the handler, report... */
		try {
			std::rethrow_exception(u.except);
		} catch (const std::exception& e) {
			cUtils::PrintErr("{}: {}", u.name, e.what());
		} catch (...) {
			cUtils::PrintErr("{}: {}", u.name, "An exception occurred");
		}
	}
	/* ...and send error message to the user */
	co_await InteractionSendMessage(i, cMessageParams()
		.SetFlags(MESSAGE_FLAG_EPHEMERAL)
		.SetContent("An unexpected error has occurred. Try again later.")
	);
}

cTask<>
cGreekBot::process_interaction(cAppCmdInteraction& i) {
	switch (i.GetCommandId().ToInt()) {
		case 878391425568473098: // avatar
			return process_avatar(i);
		case 938199801420456066: // rank
			return process_rank(i);
		case 938863857466757131: // top
			return process_top(i);
		case 1020026874119864381: // prune
			return *i.GetGuildId() == LMG_GUILD_ID ? process_prune_lmg(i) : process_prune(i);
		case 1031907652541890621: // ban
			return process_ban(i);
		case 904462004071313448: // holy
			return process_starboard_leaderboard(i);
		case 1177042125205024868: // timestamp
			return process_timestamp(i);
		case 1170787836434317363: // Apps > Ban
			return process_ban_ctx_menu(i, SUBCMD_USER);
		case 1174826008474570892: // Apps > Ban Turk
			return process_ban_ctx_menu(i, SUBCMD_TURK);
		case 1174836455714078740: // Apps > Ban Greek
			return process_ban_ctx_menu(i, SUBCMD_GREEK);
		default:
			return [](const cAppCmdInteraction& i) -> cTask<> {
				cUtils::PrintErr("Unhandled command: {} {}", i.GetCommandName(), i.GetCommandId());
				co_return;
			}(i);
	}
}

cTask<>
cGreekBot::process_interaction(cMsgCompInteraction& i) {
	const std::string_view custom_id = i.GetCustomId();
	switch (const uint32_t hash = cUtils::CRC32(0, custom_id)) {
		case CMP_ID_LEADERBOARD_HELP:
			co_await process_leaderboard_help(i);
			break;
		case CMP_ID_STARBOARD_HELP:
			co_await process_starboard_help(i);
			break;
		case CMP_ID_PROFICIENCY_MENU:
			co_await process_proficiency_menu(i);
			break;
		case CMP_ID_BOOSTER_MENU:
			co_await process_booster_menu(i);
			break;
		default:
			if (custom_id.starts_with("BAN#"))
				co_await process_unban(i, custom_id.substr(4));
			else if (custom_id.starts_with("DLT#"))
				co_await process_dismiss(i, custom_id.substr(4));
			else if (custom_id.starts_with("NCK#"))
				co_await process_nickname_button(i, custom_id.substr(4));
			else
				co_await process_role_button(i, hash); // TODO: Make custom_id use one specific prefix for roles
			break;
	}
}

cTask<>
cGreekBot::process_interaction(cModalSubmitInteraction& i) {
	std::string_view custom_id = i.GetCustomId();
	if (custom_id.starts_with("ban:"))
		return process_ban_modal(i);
	if (custom_id == "NICKNAME_MODAL")
		return process_modal(i);

	return [](std::string_view id) -> cTask<> {
		cUtils::PrintErr("Unknown modal id: {} 0x{:08X}", id, cUtils::CRC32(0, id));
		co_return;
	} (custom_id);
}