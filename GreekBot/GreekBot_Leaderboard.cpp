#include "GreekBot.h"
#include "Database.h"

void
cGreekBot::OnInteraction_rank(chInteraction interaction) {
	/* Retrieve user and member data */
	auto data = interaction->GetData<INTERACTION_APPLICATION_COMMAND>();
	chUser user;
	chMember member;
	if (data->Options.empty()) {
		member = interaction->GetMember();
		user = member->GetUser();
	}
	else {
		user = data->Options[0]->GetValue<APP_COMMAND_OPT_USER>(member);
	}

	/* Don't display data for bot users */
	if (user->IsBotUser()) {
		RespondToInteraction(interaction, "Ranking isn't available for bot users.", MESSAGE_FLAG_NONE, nullptr, nullptr, nullptr, nullptr);
		return;
	}
	if (user->IsSystemUser()) {
		RespondToInteraction(interaction, "Ranking isn't available for system users.", MESSAGE_FLAG_NONE, nullptr, nullptr, nullptr, nullptr);
		return;
	}
	/* Acknowledge interaction while we're looking through the database */
	AcknowledgeInteraction(interaction);
	/* Sort guild_roles based on position */
	if (m_lmg_sorted_roles.empty()) {
		for (auto& r : m_lmg_roles)
			m_lmg_sorted_roles.push_back(&r);
		std::sort(m_lmg_sorted_roles.begin(), m_lmg_sorted_roles.end(), [](chRole a, chRole b) { return a->GetPosition() > b->GetPosition(); });
	}
	/* Get user's ranking info from the database */
	tRankQueryData db_result;
	if (!cDatabase::GetUserRank(user, db_result)) {
		EditInteractionResponse(interaction, "Hmm... Looks like I've run into some trouble. Try again later!", MESSAGE_FLAG_EPHEMERAL, nullptr, nullptr, nullptr, nullptr);
		return;
	}
	/* Calculate level */
	if (db_result.empty()) {
		/* User not registered in the leaderboard */
		EditInteractionResponse(
			interaction, nullptr, MESSAGE_FLAG_NONE,
			std::vector<cEmbed> {
				cEmbed::CreateBuilder()
					.SetAuthor(cUtils::Format("%s#%s", user->GetUsername(), user->GetDiscriminator()).c_str(), nullptr, user->GetAvatarUrl())
					.SetDescription(member ? "User has no XP yet." : "User is not a member of **Learning Greek**.")
					.SetColor(member ? GetGuildMemberColor(*interaction->GetGuildId(), member, user) : cColor(0x0092FF))
					.Build()
			}, nullptr, nullptr, nullptr
		);
		return;
	}
	/* User data found in the database, but first check if they're still a member of Learning Greek */
	if (!member) {
		EditInteractionResponse(
			interaction, nullptr, MESSAGE_FLAG_NONE,
			std::vector<cEmbed> {
				cEmbed::CreateBuilder()
				.SetAuthor(cUtils::Format("%s#%s", user->GetUsername(), user->GetDiscriminator()).c_str(), nullptr, user->GetAvatarUrl())
				.SetDescription("User is not a member of **Learning Greek** anymore.")
				.SetColor(0x0092ff)
				.Build()
			}, nullptr, nullptr, nullptr
		);
		return;
	}
	auto& res = db_result[0];
	for (int64_t curr_lvl = 0, curr_xp = 0, next_lvl = 1, next_xp;;) {
		next_xp = curr_xp + 5 * (next_lvl * next_lvl) + 40 * (next_lvl) + 55;
		if (next_xp > db_result[0].GetXp()) {
			/* Prepare embed title */
			const char* medal;
			switch (res.GetRank()) {
				case 1:  medal = "🥇"; break;
				case 2:  medal = "🥈"; break;
				case 3:  medal = "🥉"; break;
				default: medal = "🏅"; break;
			}
			EditInteractionResponse(
				interaction, nullptr, MESSAGE_FLAG_NONE,
				std::vector<cEmbed> {
					cEmbed::CreateBuilder()
						.SetAuthor(cUtils::Format("%s#%s", user->GetUsername(), user->GetDiscriminator()).c_str(), nullptr, user->GetAvatarUrl())
						.SetTitle(cUtils::Format("%s Rank **#%" PRIi64 "**    Level **%" PRIi64 "**", medal, res.GetRank(), curr_lvl).c_str())
						.SetColor(GetGuildMemberColor(m_lmg_id, member, user))
						//.AddField("Level", std::to_string(curr_lvl).c_str(), true)
						.AddField("XP Progress", cUtils::Format("%" PRIi64 "/%" PRIi64, res.GetXp() - curr_xp, next_xp - curr_xp).c_str(), true)
						.AddField("Total XP", std::to_string(res.GetXp()).c_str(), true)
						.AddField("Messages", std::to_string(res.GetNumMessages()).c_str(), true)
						//.SetFooter("Keep talking and establish yourself in the leaderboard! aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", nullptr)
						.Build()
				},
				nullptr,
				std::vector<cActionRow> {
					cActionRow {
						cButton<BUTTON_STYLE_SECONDARY> {
							STR(CMP_ID_BUTTON_RANK_HELP),
							"How does this work?"
						}
					}
				},
				nullptr
			);
			return;
		}
		curr_xp = next_xp;
		curr_lvl++;
		next_lvl++;
	}
}

void
cGreekBot::OnInteraction_top(chInteraction interaction) {
	/* Acknowledge interaction */
	AcknowledgeInteraction(interaction);
	/* Sort guild roles based on position */
	if (m_lmg_sorted_roles.empty()) {
		for (auto& r : m_lmg_roles)
			m_lmg_sorted_roles.push_back(&r);
		std::sort(m_lmg_sorted_roles.begin(), m_lmg_sorted_roles.end(), [](chRole a, chRole b) { return a->GetPosition() > b->GetPosition(); });
	}
	/* Get data from the database */
	tRankQueryData db_result;
	if (!cDatabase::GetTop10(db_result)) {
		EditInteractionResponse(interaction, "Hmm... Looks like I've run into some trouble. Try again later!", MESSAGE_FLAG_NONE, nullptr, nullptr, nullptr, nullptr);
		return;
	}
	if (db_result.empty()) {
		EditInteractionResponse(interaction, "I don't have any data yet. Start talking!", MESSAGE_FLAG_NONE, nullptr, nullptr, nullptr, nullptr);
		return;
	}
	/* Display data */
	// TODO: make this pretty with embeds and stuff...
	std::string str;
	std::vector<cEmbed> embeds;
	embeds.reserve(db_result.size());
	uchMember u_member;
	uchUser   u_user;
	chMember  member;
	chUser    user;
	for (auto &d: db_result) {
		if (d.GetUserId()->ToInt() == interaction->GetMember()->GetUser()->GetId()->ToInt()) {
			member = interaction->GetMember();
			user = member->GetUser();
		}
		else if ((u_member = GetGuildMember(m_lmg_id, d.GetUserId()))) {
			/* Try to resolve member object */
			member = u_member.get();
			user = member->GetUser();
		}
		else {
			/* If user isn't a member anymore... TBA */
		}

		cColor color = 0;
		for (auto& r : m_lmg_sorted_roles) {
			if (std::find_if(member->Roles.begin(), member->Roles.end(), [&r](const chSnowflake& a) { return r->GetId()->ToInt() == a->ToInt();}) != member->Roles.end()) {
				if (r->GetColor().ToInt() != 0) {
					color = r->GetColor();
					break;
				}
			}
		}

		embeds.emplace_back(
			cEmbed::CreateBuilder()
				.SetAuthor(cUtils::Format("%s#%s", user->GetUsername(), user->GetDiscriminator()).c_str(), nullptr, user->GetAvatarUrl())
				.SetTitle(cUtils::Format("🏅 Rank **#%d**\tLevel **TBA**", (int)d.GetRank()).c_str())
				.SetColor(color/*GetGuildMemberColor(m_lmg_id, member.get(), user)*/)
				.AddField("XP Progress", "TBA", true)
				.AddField("Total XP", std::to_string(d.GetXp()).c_str(), true)
				.AddField("Messages", std::to_string(d.GetNumMessages()).c_str(), true)
				.Build()
		);
		/*
		char test[500];
		sprintf(test, "Rank #%d, User id: %s", (int)d.GetRank(), d.GetUserId()->ToString());
		str += test;
		str += '\n';*/
	}
	//auto e = cEmbed::CreateBuilder().SetDescription("MEOW").AddField("A", "B").AddField("C", "D").Build();
	//cEmbed es[] { e, e, e, e, e, e, e, e, e, e };
	EditInteractionResponse(interaction, str.c_str(), MESSAGE_FLAG_NONE, embeds, nullptr, nullptr, nullptr);
}
