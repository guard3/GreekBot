#include "GreekBot.h"
#include "Database.h"

/* Helper functions that create embeds */
static cEmbed make_no_xp_embed(const cUser& user, cColor c) {
	return {
		author={
			user.GetUsername() + '#' + user.GetDiscriminator(),
			icon_url=user.GetAvatarUrl()
		},
		description="User has no XP yet.",
		color=c
	};
}
static cEmbed make_no_member_embed(const cUser& user, bool bAnymore) {
	return {
		author={
			user.GetUsername() + '#' + user.GetDiscriminator(),
			icon_url=user.GetAvatarUrl()
		},
		description=cUtils::Format("User is not a member of **Learning Greek**%s", bAnymore ? " anymore." : "."),
		color=0x0096FF
	};
}
static cEmbed make_embed(const cUser& user, const cMember& member, cColor c, int64_t rank, int64_t xp, int64_t num_msg) {
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
	return {
		author={
			user.GetUsername() + '#' + user.GetDiscriminator(),
			icon_url = user.GetAvatarUrl()
		},
		title=cUtils::Format("%s Rank **#%" PRIi64 "**\tLevel **%" PRIi64 "**", medal, rank, level),
		color=c,
		fields={
			{ "XP Progress", cUtils::Format("%" PRIi64 "/%" PRIi64, xp - base_xp, next_xp - base_xp), true},
			{ "Total XP", std::to_string(xp), true },
			{ "Messages", std::to_string(num_msg), true }
		}
	};
}

cTask<>
cGreekBot::OnInteraction_rank(const cInteraction& i) {
	try {
		/* Resolve user and member data */
		auto data = i.GetData<INTERACTION_APPLICATION_COMMAND>();
		chUser user;
		chMember member;
		if (data->Options.empty()) {
			member = i.GetMember();
			user = member->GetUser();
		} else {
			user = &data->Options[0].GetValue<APP_CMD_OPT_USER>();
			member = data->Options[0].GetMember();
		}
		/* Don't display data for bot users */
		if (user->IsBotUser())
			co_return co_await RespondToInteraction(i, content="Ranking isn't available for bot users.");
		if (user->IsSystemUser())
			co_return co_await RespondToInteraction(i, content="Ranking isn't available for system users.");
		/* Acknowledge interaction while we're looking through the database */
		co_await RespondToInteraction(i);
		/* Get user's ranking info from the database */
		tRankQueryData db_result = co_await cDatabase::GetUserRank(*user);
		co_await ResumeOnEventThread();
		/* Make sure that the selected user is a member of Learning Greek */
		if (!member) {
			co_await EditInteractionResponse(i, embeds={make_no_member_embed(*user, !db_result.empty())});
			co_return;
		}
		/* Respond */
		cColor color = get_lmg_member_color(*member);
		if (db_result.empty()) {
			/* User not registered in the leaderboard */
			co_await EditInteractionResponse(i, embeds={make_no_xp_embed(*user, color)});//MESSAGE_FLAG_NONE, {.embeds {make_no_xp_embed(*user, color)}});
		} else {
			/* Respond to interaction with a proper embed */
			auto &res = db_result[0];
			co_await EditInteractionResponse(i,
				embeds={ make_embed(*user, *member, color, res.GetRank(), res.GetXp(), res.GetNumMessages()) },
				components={
					cActionRow{
						cButton<BUTTON_STYLE_SECONDARY>{
							STR(CMP_ID_BUTTON_RANK_HELP),
							label="How does this work?"
						}
					}
				}
			);
		}
	}
	catch (const std::exception& e) {
		cUtils::PrintErr("OnInteraction_rank: %s", e.what());
	}
}

cTask<>
cGreekBot::OnInteraction_top(const cInteraction& i) {
	try {
		/* Acknowledge interaction */
		co_await RespondToInteraction(i);
		/* Get data from the database */
		tRankQueryData db_result = co_await cDatabase::GetTop10();
		if (db_result.empty()) {
			co_await EditInteractionResponse(i, content="I don't have any data yet. Start talking!");
			co_return;
		}
		/* Get members */
		std::vector<cSnowflake> ids;
		ids.reserve(db_result.size());
		for (auto& r : db_result)
			ids.emplace_back(*r.GetUserId());
		std::vector<cMember> members;
		members.reserve(db_result.size());
		for (auto gen = GetGuildMembersById(*i.GetGuildId(), ids); co_await gen.HasValue();)
			members.emplace_back(co_await gen());
		/* Prepare embeds */
		std::vector<cEmbed> es;
		es.reserve(db_result.size());
		for (auto &d: db_result) {
			auto m = std::find_if(members.begin(), members.end(), [&](const cMember& k) { return k.GetUser()->GetId() == *d.GetUserId(); });
			if (m == members.end())
				// TODO: if GetUser() fails?
				es.push_back(make_no_member_embed(co_await GetUser(*d.GetUserId()), true));
			else
				es.push_back(make_embed(*m->GetUser(), *m, get_lmg_member_color(*m), d.GetRank(), d.GetXp(), d.GetNumMessages()));
		}
		/* Respond to interaction */
		co_await EditInteractionResponse(i,
			embeds=std::move(es),
			components={
				cActionRow{
					cButton<BUTTON_STYLE_SECONDARY>{
						STR(CMP_ID_BUTTON_RANK_HELP),
						label="How does this work?"
					}
				}
			}
		);
	}
	catch (const std::exception& e) {
		cUtils::PrintErr("OnInteraction_top: %s", e.what());
	}
}