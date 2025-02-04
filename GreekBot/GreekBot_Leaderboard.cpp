#include "GreekBot.h"
#include "DBLeaderboard.h"
#include "CDN.h"
#include "Utils.h"
#include <algorithm>
#include <ranges>
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
static void make_no_member_embed(cEmbed& embed, const cUser* pUser, std::string_view guild_name, bool bAnymore) {
	embed.SetDescription(std::format("User is not a member of **{}**{}.", guild_name, bAnymore ? " anymore": ""));
	embed.SetColor(LMG_COLOR_BLUE);
	if (pUser)
		embed.EmplaceAuthor(pUser->GetUsername()).SetIconUrl(cCDN::GetUserAvatar(*pUser));
	else
		embed.EmplaceAuthor("Deleted user").SetIconUrl(cCDN::GetDefaultUserAvatar(cSnowflake{}));
}
static void make_embed(cEmbed& embed, const cUser& user, cColor c, const leaderboard_entry* pLb) {
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
	embed.SetTitle(std::format("{} Rank **#{}**\tLevel **{}**", medal, pLb->rank, level));
	embed.SetFields({
		{ "XP Progress", std::format("{}/{}", pLb->xp - level_xp, next_level_xp - level_xp), true},
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
	auto db = co_await BorrowDatabase();
	auto xp = cLeaderboardDAO(db).Update(msg);
	co_await ReturnDatabase(std::move(db));
	/* TODO: this returns 0 unexpectedly! */
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
	cPartialMessage response;
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
	auto db = co_await BorrowDatabase();
	auto db_result = cLeaderboardDAO(db).GetEntryByUser(*user);
	co_await ReturnDatabase(std::move(db));
	co_await ResumeOnEventStrand();
	/* Make sure that the selected user is a member of Learning Greek */
	auto& embeds = response.EmplaceEmbeds();
	if (cEmbed& embed = embeds.emplace_back(); member)
		make_embed(embed, *user, get_lmg_member_color(*member), db_result ? &*db_result : nullptr);
	else
		make_no_member_embed(embed, user.Get(), m_guilds.at(LMG_GUILD_ID).GetName(), db_result.has_value());
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
	/* Acknowledge interaction; accessing the database may be slow */
	co_await InteractionDefer(i);

	/* Get data from the database */
	auto db = co_await BorrowDatabase();
	std::vector lb_entries = cLeaderboardDAO(db).GetTop10Entries();
	co_await ReturnDatabase(std::move(db));

	cPartialMessage response;
	if (lb_entries.empty())
		co_return co_await InteractionSendMessage(i, response.SetContent("I don't have any data yet. Start talking!"));

	/* Collect members from the leaderboard entries */
	std::vector<cMember> members; {
		members.reserve(lb_entries.size());
		std::vector member_ids = lb_entries | std::views::transform(&leaderboard_entry::user_id) | std::ranges::to<std::vector>();
		co_for (cMember& mem, RequestGuildMembers(*i.GetGuildId(), member_ids))
			members.push_back(std::move(mem));
	}

	/* Prepare embeds */
	auto& embeds = response.EmplaceEmbeds();
	embeds.reserve(lb_entries.size());
	co_await ResumeOnEventStrand();

	/* Create embeds for every leaderboard entry */
	for (auto& lb_entry: lb_entries) {
		cEmbed& embed = embeds.emplace_back();
		if (auto it = rng::find(members, lb_entry.user_id, [](const cMember& m) -> const cSnowflake& { return m.GetUser()->GetId(); }); it != end(members)) {
			/* If the user is a member of Learning Greek, make a regular embed */
			make_embed(embed, *it->GetUser(), get_lmg_member_color(*it), &lb_entry);
		} else {
			/* Otherwise, make a no-member embed */
			std::optional<cUser> opt;
			cUser *pUser = nullptr;
			try {
				pUser = &opt.emplace(co_await GetUser(lb_entry.user_id));
			} catch (const xDiscordError&) {
				/* User object couldn't be retrieved, likely deleted */
			}
			make_no_member_embed(embed, pUser, m_guilds.at(*i.GetGuildId()).GetName(), true);
		}
	}

	/* Send final message */
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
	co_await InteractionSendMessage(i, cPartialMessage()
		.SetFlags(MESSAGE_FLAG_EPHEMERAL)
		.SetContent("Every minute that you're messaging, you randomly gain between 15 and 25 **XP**.")
	);
} HANDLER_END