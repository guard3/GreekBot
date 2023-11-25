#include "GreekBot.h"
#include "Database.h"
#include "Utils.h"
#include "CDN.h"

/* Helper functions that create embeds */
static cEmbed make_no_xp_embed(const cUser& user, cColor c) {
	return cEmbed{
		kw::author={
			user.GetUsername(),
			kw::icon_url=cCDN::GetUserAvatar(user)
		},
		kw::description="User has no XP yet.",
		kw::color=c
	};
}
static cEmbed make_no_member_embed(const cUser* pUser, std::string_view guild_name, bool bAnymore) {
	return cEmbed{
		kw::author=pUser ? cEmbedAuthor{
			pUser->GetUsername(),
			kw::icon_url=cCDN::GetUserAvatar(*pUser)
		} : cEmbedAuthor{
			"Deleted user",
			kw::icon_url=cCDN::GetDefaultUserAvatar(cSnowflake())
		},
		kw::description=fmt::format("User is not a member of **{}**{}.", guild_name, bAnymore ? " anymore" : ""),
		kw::color=0x0096FF
	};
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
	return cEmbed{
		kw::author={
			user.GetUsername(),
			kw::icon_url=cCDN::GetUserAvatar(user)
		},
		kw::title=fmt::format("{} Rank **#{}**\tLevel **{}**", medal, rank, level),
		kw::color=c,
		kw::fields={
			{ "XP Progress", fmt::format("{}/{}", xp - base_xp, next_xp - base_xp), true},
			{ "Total XP", std::to_string(xp), true },
			{ "Messages", std::to_string(num_msg), true }
		}
	};
}

cTask<>
cGreekBot::process_rank(cAppCmdInteraction& i) {
	try {
		/* Resolve user and member data */
		hUser user;
		hPartialMember member;
		if (auto options = i.GetOptions(); !options.empty()) {
			std::tie(user, member) = options.front().GetValue<APP_CMD_OPT_USER>();
		} else {
			user = &i.GetUser();
			member = i.GetMember();
		}
		/* Don't display data for bot users */
		if (user->IsBotUser())
			co_return co_await RespondToInteraction(i, kw::content="Ranking isn't available for bot users.");
		if (user->IsSystemUser())
			co_return co_await RespondToInteraction(i, kw::content="Ranking isn't available for system users.");
		/* Acknowledge interaction while we're looking through the database */
		co_await RespondToInteraction(i);
		/* Get user's ranking info from the database */
		tRankQueryData db_result = co_await cDatabase::GetUserRank(*user);
		co_await ResumeOnEventThread();
		/* Make sure that the selected user is a member of Learning Greek */
		if (!member)
			co_return co_await EditInteractionResponse(i, kw::embeds={ make_no_member_embed(user.Get(), m_guilds.at(m_lmg_id)->GetName(), !db_result.empty()) });
		/* Respond */
		cColor color = get_lmg_member_color(*member);
		if (db_result.empty()) {
			/* User not registered in the leaderboard */
			co_await EditInteractionResponse(i, kw::embeds={make_no_xp_embed(*user, color)});
		} else {
			/* Respond to interaction with a proper embed */
			auto &res = db_result[0];
			co_await EditInteractionResponse(i,
				kw::embeds={ make_embed(*user, color, res.GetRank(), res.GetXp(), res.GetNumMessages()) },
				kw::components={
					cActionRow{
						cButton{
							BUTTON_STYLE_SECONDARY,
							"LEADERBOARD_HELP",
							kw::label="How does this work?"
						}
					}
				}
			);
		}
	}
	catch (const std::exception& e) {
		cUtils::PrintErr("OnInteraction_rank: {}", e.what());
	}
}

cTask<>
cGreekBot::process_top(cAppCmdInteraction& i) {
	try {
		/* Acknowledge interaction */
		co_await RespondToInteraction(i);
		/* Get data from the database */
		tRankQueryData db_result = co_await cDatabase::GetTop10();
		co_await ResumeOnEventThread();
		if (db_result.empty()) {
			co_await EditInteractionResponse(i, kw::content="I don't have any data yet. Start talking!");
			co_return;
		}
		/* Get members */
		std::vector<cSnowflake> ids;
		ids.reserve(db_result.size());
		for (auto& r : db_result)
			ids.emplace_back(*r.GetUserId());
		std::vector<cMember> members;
		members.reserve(db_result.size());
		for (auto gen = GetGuildMembers(*i.GetGuildId(), kw::user_ids=ids); co_await gen.HasValue();)
			members.emplace_back(co_await gen());
		/* Prepare embeds */
		std::vector<cEmbed> es;
		es.reserve(db_result.size());
		for (auto &d: db_result) {
			/* If the user is still a member of Learning Greek, make a regular embed */
			auto m = std::find_if(members.begin(), members.end(), [&](const cMember& k) { return k.GetUser()->GetId() == *d.GetUserId(); });
			if (m != members.end()) {
				es.push_back(make_embed(*m->GetUser(), get_lmg_member_color(*m), d.GetRank(), d.GetXp(), d.GetNumMessages()));
				continue;
			}
			/* Otherwise, make a no-member embed */
			std::string_view guild_name = m_guilds.at(m_lmg_id)->GetName();
			try {
				cUser user = co_await GetUser(*d.GetUserId());
				es.push_back(make_no_member_embed(&user, guild_name, true));
			} catch (const xDiscordError& e) {
				/* User object couldn't be retrieved, likely deleted */
				es.push_back(make_no_member_embed(nullptr, guild_name, true));
			}
		}
		/* Respond to interaction */
		co_await EditInteractionResponse(i,
			kw::embeds=std::move(es),
			kw::components={
				cActionRow{
					cButton{
						BUTTON_STYLE_SECONDARY,
						"LEADERBOARD_HELP",
						kw::label="How does this work?"
					}
				}
			}
		);
	}
	catch (const std::exception& e) {
		cUtils::PrintErr("OnInteraction_top: {}", e.what());
	}
}

cTask<>
cGreekBot::process_leaderboard_help(cMsgCompInteraction& i) {
	co_await RespondToInteraction(i);
	co_await SendInteractionFollowupMessage(i, kw::flags=MESSAGE_FLAG_EPHEMERAL, kw::content="Every minute that you're messaging, you randomly gain between 15 and 25 **XP**.");
}