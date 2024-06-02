#include "GreekBot.h"
#include "Database.h"
#include "CDN.h"

/* Helper functions that create embeds */
static cEmbed make_no_xp_embed(const cUser& user, cColor c) {
	cEmbed embed;
	embed.EmplaceAuthor(user.GetUsername()).SetIconUrl(cCDN::GetUserAvatar(user));
	embed.SetDescription("User has no XP yet.");
	embed.SetColor(c);
	return embed;
}
static cEmbed make_no_member_embed(const cUser* pUser, std::string_view guild_name, bool bAnymore) {
	cEmbed embed;
	embed.SetDescription(fmt::format("User is not a member of **{}**{}.", guild_name, bAnymore ? " anymore": ""));
	embed.SetColor(0x0096FF);
	if (pUser)
		embed.EmplaceAuthor(pUser->GetUsername()).SetIconUrl(cCDN::GetUserAvatar(*pUser));
	else
		embed.EmplaceAuthor("Deleted user").SetIconUrl(cCDN::GetDefaultUserAvatar(cSnowflake{}));
	return embed;
}
static cEmbed make_embed(const cUser& user, cColor c, int64_t rank, int64_t xp, int64_t num_msg) {
	/* Choose a medal emoji depending on the user's rank */
	const char* medal;
	switch (rank) {
		case 1:  medal = "🥇"; break;
		case 2:  medal = "🥈"; break;
		case 3:  medal = "🥉"; break;
		default: medal = "🏅"; break;
	}
	/* Resolve user's leaderboard level */
	int64_t level = 0, base_xp = 0, next_xp = 100;
	while (next_xp < xp) {
		level++;
		base_xp = next_xp;
		next_xp += 5 * level * level + 50 * level + 100;
	}
	/* Create embed */
	cEmbed embed;
	embed.EmplaceAuthor(user.GetUsername()).SetIconUrl(cCDN::GetUserAvatar(user));
	embed.SetTitle(fmt::format("{} Rank **#{}**\tLevel **{}**", medal, rank, level));
	embed.SetColor(c);
	embed.SetFields({
		{ "XP Progress", fmt::format("{}/{}", xp - base_xp, next_xp - base_xp), true},
		{ "Total XP", std::to_string(xp), true },
		{ "Messages", std::to_string(num_msg), true }
	});
	return embed;
}

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
	tRankQueryData db_result = co_await cDatabase::GetUserRank(*user);
	co_await ResumeOnEventThread();
	/* Make sure that the selected user is a member of Learning Greek */
	if (!member)
		co_return co_await InteractionSendMessage(i, response.SetEmbeds({
			make_no_member_embed(user.Get(), m_guilds.at(LMG_GUILD_ID)->GetName(), !db_result.empty())
		}));
	/* Respond */
	cColor color = get_lmg_member_color(*member);
	if (db_result.empty()) {
		/* User not registered in the leaderboard */
		co_await InteractionSendMessage(i, response.SetEmbeds({
			make_no_xp_embed(*user, color)
		}));
	} else {
		/* Respond to interaction with a proper embed */
		auto& res = db_result.front();
		co_await InteractionSendMessage(i, response
			.SetEmbeds({
				make_embed(*user, color, res.GetRank(), res.GetXp(), res.GetNumMessages())
			})
			.SetComponents({
				cActionRow{
					cButton{
						BUTTON_STYLE_SECONDARY,
						"LEADERBOARD_HELP",
						"How does this work?"
					}
				}
			})
		);
	}
} HANDLER_END

cTask<>
cGreekBot::process_top(cAppCmdInteraction& i) HANDLER_BEGIN {
	/* Prepare the response */
	cMessageParams response;
	/* Acknowledge interaction */
	co_await InteractionDefer(i);
	/* Get data from the database */
	tRankQueryData db_result = co_await cDatabase::GetTop10();
	co_await ResumeOnEventThread();
	if (db_result.empty())
		co_return co_await InteractionSendMessage(i, response.SetContent("I don't have any data yet. Start talking!"));
	/* Get members */
	std::vector<cSnowflake> ids;
	ids.reserve(db_result.size());
	for (auto& r : db_result)
		ids.emplace_back(*r.GetUserId());
	std::vector<cMember> members;
	members.reserve(db_result.size());
	auto gen = GetGuildMembers(*i.GetGuildId(), kw::user_ids=ids);
	for (auto it = co_await gen.begin(); it != gen.end(); co_await ++it)
		members.push_back(std::move(*it));
	/* Prepare embeds */
	auto& embeds = response.EmplaceEmbeds();
	embeds.reserve(db_result.size());
	for (auto &d: db_result) {
		/* If the user is still a member of Learning Greek, make a regular embed */
		auto m = std::find_if(members.begin(), members.end(), [&](const cMember& k) { return k.GetUser()->GetId() == *d.GetUserId(); });
		if (m != members.end()) {
			embeds.push_back(make_embed(*m->GetUser(), get_lmg_member_color(*m), d.GetRank(), d.GetXp(), d.GetNumMessages()));
			continue;
		}
		/* Otherwise, make a no-member embed */
		std::optional<cUser> opt;
		cUser* pUser = nullptr;
		try {
			pUser = &opt.emplace(co_await GetUser(*d.GetUserId()));
		} catch (const xDiscordError&) {
			/* User object couldn't be retrieved, likely deleted */
		}
		embeds.push_back(make_no_member_embed(pUser, m_guilds.at(LMG_GUILD_ID)->GetName(), true));
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