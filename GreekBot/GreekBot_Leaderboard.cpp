#include "GreekBot.h"
#include "Database.h"
#include "CDN.h"
#include "Utils.h"
#include <algorithm>
/* ================================================================================================================== */
namespace rng = std::ranges;
/* ========== Helper macros to catch and report errors ============================================================== */
#define LEADERBOARD_HANDLER_BEGIN try
#define LEADERBOARD_HANDLER_END catch (const xDiscordError& e) {        \
    cUtils::PrintErr("An error occurred in {}: {}", __func__, e.what());\
    cUtils::PrintErr("Error details: {}", e.errors());                  \
}
/* ========== Level role ids ======================================================================================== */
enum : std::uint64_t {
	ROLE_ID_MEMBER               = 350264923806367754, // @Member
	ROLE_ID_ACTIVE_MEMBER        = 352008860904456192, // @Active Member
	ROLE_ID_ACTIVE_USER          = 352009155864821760, // @Active User
	ROLE_ID_REALLY_ACTIVE_MEMBER = 353354156783960075, // @Really Active Member
	ROLE_ID_USUAL_SUSPECT        = 410753563543732225, // @Usual Suspect
	ROLE_ID_GREEK_LOVER          = 410754841556549633, // @Greek Lover
	ROLE_ID_PROESTOS             = 414523335595130905, // @Προεστός
	ROLE_ID_EPITIMOS             = 466238555304230913, // @Επίτιμος
	ROLE_ID_ELLINOPOULO          = 608687509291008000, // @Ελληνόπουλο
	ROLE_ID_PALIOS               = 631559446782410752  // @Παλιός
};
/* ========== Helper function to calculate a member's level info based on their XP count ============================ */
struct level_info {
	std::uint64_t level;         // The current level
	std::uint64_t level_xp;      // The minimum XP required to reach current level
	std::uint64_t next_level_xp; // The XP required to reach the next level
};
static level_info calculate_level_info(std::uint64_t xp) noexcept {
	level_info result{ 0, 0, 100 };
	while (result.next_level_xp < xp) {
		result.level++;
		result.level_xp = result.next_level_xp;
		result.next_level_xp += 5 * result.level * (result.level + 10) + 100;
	}
	return result;
}
/* ========== Helper functions that create embeds =================================================================== */
static void insert_no_member_embed(std::vector<cEmbed>& embeds, const cUser* pUser, std::string_view guild_name, bool bAnymore) {
	auto& embed = embeds.emplace_back();
	embed.SetDescription(fmt::format("User is not a member of **{}**{}.", guild_name, bAnymore ? " anymore": ""));
	embed.SetColor(0x0096FF);
	if (pUser)
		embed.EmplaceAuthor(pUser->GetUsername()).SetIconUrl(cCDN::GetUserAvatar(*pUser));
	else
		embed.EmplaceAuthor("Deleted user").SetIconUrl(cCDN::GetDefaultUserAvatar(cSnowflake{}));
}
static void insert_embed(std::vector<cEmbed>& embeds, const cUser& user, cColor c, const leaderboard_entry* pLb) {
	auto& embed = embeds.emplace_back();
	embed.EmplaceAuthor(user.GetUsername()).SetIconUrl(cCDN::GetUserAvatar(user));
	embed.SetColor(c);
	if (!pLb) {
		embed.SetDescription("User has no XP yet.");
		return;
	}
	/* Choose a medal emoji depending on the user's rank */
	const char* medal;
	switch (pLb->rank) {
		case 1:  medal = "🥇"; break;
		case 2:  medal = "🥈"; break;
		case 3:  medal = "🥉"; break;
		default: medal = "🏅"; break;
	}
	/* Resolve user's leaderboard level */
	auto[level, level_xp, next_level_xp] = calculate_level_info(pLb->xp);
	/* Create embed */
	embed.SetTitle(fmt::format("{} Rank **#{}**\tLevel **{}**", medal, pLb->rank, level));
	embed.SetFields({
		{ "XP Progress", fmt::format("{}/{}", pLb->xp - level_xp, next_level_xp - level_xp), true},
		{ "Total XP", std::to_string(pLb->xp), true },
		{ "Messages", std::to_string(pLb->num_msg), true }
	});
}

cTask<>
cGreekBot::process_leaderboard_new_message(cMessage& msg, cPartialMember& member) LEADERBOARD_HANDLER_BEGIN {
	static constexpr std::uint64_t rank_role_ids[] {
		ROLE_ID_MEMBER,
		ROLE_ID_ACTIVE_MEMBER,
		ROLE_ID_ACTIVE_USER,
		ROLE_ID_REALLY_ACTIVE_MEMBER,
		ROLE_ID_USUAL_SUSPECT,
		ROLE_ID_GREEK_LOVER,
		ROLE_ID_PROESTOS,
		ROLE_ID_EPITIMOS,
		ROLE_ID_ELLINOPOULO,
		ROLE_ID_PALIOS
	};
	static_assert(rng::is_sorted(rank_role_ids), "Must be sorted for binary search");
	/* Update leaderboard and retrieve the author's total xp */
	std::uint64_t xp = co_await cDatabase::UpdateLeaderboard(msg);
	if (xp == 0)
		co_return;
	/* Calculate member level from xp */
	auto[level, level_xp, next_level_xp] = calculate_level_info(xp);
	/* Get the appropriate rank role for the current level, or 0 for no role */
	std::uint64_t target_role_id;
	if (level >= 100) {
		target_role_id = ROLE_ID_PALIOS;
	} else if (level >= 70) {
		target_role_id = ROLE_ID_ELLINOPOULO;
	} else if (level >= 50) {
		target_role_id = ROLE_ID_EPITIMOS;
	} else if (level >= 40) {
		target_role_id = ROLE_ID_PROESTOS;
	} else if (level >= 35) {
		target_role_id = ROLE_ID_GREEK_LOVER;
	} else if (level >= 25) {
		target_role_id = ROLE_ID_USUAL_SUSPECT;
	} else if (level >= 20) {
		target_role_id = ROLE_ID_REALLY_ACTIVE_MEMBER;
	} else if (level >= 15) {
		target_role_id = ROLE_ID_ACTIVE_MEMBER;
	} else if (level >= 10) {
		target_role_id = ROLE_ID_MEMBER;
	} else if (level >= 6) {
		target_role_id = ROLE_ID_ACTIVE_USER;
	} else {
		target_role_id = 0;
	}
	/* Prepare member options with the member's roles */
	cMemberOptions options;
	auto& member_roles = options.EmplaceRoles(member.MoveRoles());
	/* Push rank roles to the end of the role vector */
	auto rank_roles = rng::partition(member_roles, [](const cSnowflake& role_id) {
		return !rng::binary_search(rank_role_ids, role_id);
	});
	/* Check the role_ids to be deleted */
	switch (rank_roles.size()) {
		case 0:
			if (target_role_id == 0)
				co_return;
			/* Member should have a rank role, so we give them one */
			member_roles.emplace_back(target_role_id);
			break;
		case 1:
			if (target_role_id == 0) {
				/* Member shouldn't have any rank roles, so we remove them */
				member_roles.erase(rank_roles.begin(), rank_roles.end());
				break;
			}
			if (auto& role_id = rank_roles.front(); role_id != target_role_id) {
				/* Member has the wrong rank role, so we update it */
				role_id = target_role_id;
				break;
			}
			co_return;
		default: {
			/* We keep the correct rank role and delete the rest */
			auto it = rank_roles.begin();
			if (target_role_id != 0)
				*it++ = target_role_id;
			member_roles.erase(it, member_roles.end());
			break;
		}
	}
	co_await ModifyGuildMember(LMG_GUILD_ID, msg.GetAuthor().GetId(), options);
} LEADERBOARD_HANDLER_END

cTask<>
cGreekBot::process_rank(cAppCmdInteraction& i) HANDLER_BEGIN {
	/* Resolve user and member data */
	hUser user;
	hPartialMember member;
	if (auto options = i.GetOptions(); !options.empty()) {
		std::tie(user, member) = options.front().GetValue<APP_CMD_OPT_USER>();
	} else {
		user = &i.GetUser();
		member = i.GetMember();
	}
	/* Prepare the response */
	cMessageParams response;
	response.SetFlags(MESSAGE_FLAG_EPHEMERAL);
	/* Don't display data for bot users */
	if (user->IsBotUser())
		co_return co_await InteractionSendMessage(i, response.SetContent("Ranking isn't available for bot users."));
	if (user->IsSystemUser())
		co_return co_await InteractionSendMessage(i, response.SetContent("Ranking isn't available for system users."));
	response.ResetFlags();
	/* Acknowledge interaction while we're looking through the database */
	co_await InteractionDefer(i, true);
	/* Get user's ranking info from the database */
	auto db_result = co_await cDatabase::GetUserRank(*user);
	co_await ResumeOnEventThread();
	/* Make sure that the selected user is a member of Learning Greek */
	auto& embeds = response.EmplaceEmbeds();
	if (member)
		insert_embed(embeds, *user, get_lmg_member_color(*member), db_result ? &*db_result : nullptr);
	else
		insert_no_member_embed(embeds, user.Get(), m_guilds.at(LMG_GUILD_ID).GetName(), db_result.has_value());
	co_await InteractionSendMessage(i, response.SetComponents({
		cActionRow{
			cButton{
				BUTTON_STYLE_SECONDARY,
				"LEADERBOARD_HELP",
				"How does this work?"
			}
		}
	}));
} HANDLER_END

cTask<>
cGreekBot::process_top(cAppCmdInteraction& i) HANDLER_BEGIN {
	/* Prepare the response */
	cMessageParams response;
	/* Acknowledge interaction */
	co_await InteractionDefer(i);
	/* Get data from the database */
	auto db_result = co_await cDatabase::GetTop10();
	co_await ResumeOnEventThread();
	if (db_result.empty())
		co_return co_await InteractionSendMessage(i, response.SetContent("I don't have any data yet. Start talking!"));
	/* Get members */
	std::vector<cSnowflake> ids;
	ids.reserve(db_result.size());
	for (auto& r : db_result)
		ids.emplace_back(r.user_id);
	std::vector<cMember> members;
	members.reserve(db_result.size());
	auto gen = GetGuildMembers(*i.GetGuildId(), kw::user_ids=ids);
	for (auto it = co_await gen.begin(); it != gen.end(); co_await ++it)
		members.push_back(std::move(*it));
	/* Prepare embeds */
	auto& embeds = response.EmplaceEmbeds();
	embeds.reserve(db_result.size());
	for (auto& lb: db_result) {
		if (auto it = rng::find(members, lb.user_id, [](const cMember& m) -> const cSnowflake& { return m.GetUser()->GetId(); }); it != end(members)) {
			/* If the user is a member of Learning Greek, make a regular embed */
			insert_embed(embeds, *it->GetUser(), get_lmg_member_color(*it), &lb);
		} else {
			/* Otherwise, make a no-member embed */
			std::optional<cUser> opt;
			cUser *pUser = nullptr;
			try {
				pUser = &opt.emplace(co_await GetUser(lb.user_id));
			} catch (const xDiscordError&) {
				/* User object couldn't be retrieved, likely deleted */
			}
			insert_no_member_embed(embeds, pUser, m_guilds.at(LMG_GUILD_ID).GetName(), true);
		}
	}
	/* Respond to interaction */
	co_await InteractionSendMessage(i, response.SetComponents({
		cActionRow{
			cButton{
				BUTTON_STYLE_SECONDARY,
				"LEADERBOARD_HELP",
				"How does this work?"
			}
		}
	}));
} HANDLER_END

cTask<>
cGreekBot::process_leaderboard_help(cMsgCompInteraction& i) HANDLER_BEGIN {
	co_await InteractionSendMessage(i, cMessageParams()
		.SetFlags(MESSAGE_FLAG_EPHEMERAL)
		.SetContent("Every minute that you're messaging, you randomly gain between 15 and 25 **XP**.")
	);
} HANDLER_END