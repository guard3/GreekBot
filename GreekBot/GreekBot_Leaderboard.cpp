#include "GreekBot.h"
#include "Database.h"

/* Helper functions that create embeds */
static cEmbed make_no_xp_embed(const cUser& user, cColor color) {
	return cEmbed::CreateBuilder()
		.SetAuthor(user.GetUsername() + '#' + user.GetDiscriminator(), "", user.GetAvatarUrl())
		.SetDescription("User has no XP yet.")
		.SetColor(color)
		.Build();
}
static cEmbed make_no_member_embed(const cUser& user, bool bAnymore) {
	return cEmbed::CreateBuilder()
		.SetAuthor(user.GetUsername() + '#' + user.GetDiscriminator(), "", user.GetAvatarUrl())
		.SetDescription(cUtils::Format("User is not a member of **Learning Greek**%s", bAnymore ? " anymore." : "."))
		.SetColor(0x0096FF)
		.Build();
}
static cEmbed make_embed(const cUser& user, const cMember& member, cColor color, int64_t rank, int64_t xp, int64_t num_msg) {
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
	return cEmbed::CreateBuilder()
		.SetAuthor(user.GetUsername() + '#' + user.GetDiscriminator(), "", user.GetAvatarUrl())
		.SetTitle(cUtils::Format("%s Rank **#%" PRIi64 "**\tLevel **%" PRIi64 "**", medal, rank, level))
		.SetColor(color)
		.AddField("XP Progress", cUtils::Format("%" PRIi64 "/%" PRIi64, xp - base_xp, next_xp - base_xp), true)
		.AddField("Total XP", std::to_string(xp), true)
		.AddField("Messages", std::to_string(num_msg), true)
		.Build();
}

void
cGreekBot::OnInteraction_rank(chInteraction interaction) {
	/* Resolve user and member data */
	auto data = interaction->GetData<INTERACTION_APPLICATION_COMMAND>();
	chUser user;
	chMember member;
	if (data->Options.empty()) {
		member = interaction->GetMember();
		user = member->GetUser();
	}
	else {
		user = data->Options[0].GetValue<APP_COMMAND_OPT_USER>(member);
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
	/* Get user's ranking info from the database */
	tRankQueryData db_result;
	if (!cDatabase::GetUserRank(user, db_result)) {
		EditInteractionResponse(interaction, "Hmm... Looks like I've run into some trouble. Try again later!", MESSAGE_FLAG_EPHEMERAL, nullptr, nullptr, nullptr, nullptr);
		return;
	}
	/* Make sure that the selected user is a member of Learning Greek */
	if (!member) {
		EditInteractionResponse(
			interaction, nullptr, MESSAGE_FLAG_NONE,
			std::vector<cEmbed> { make_no_member_embed(*user, !db_result.empty()) },
			nullptr, nullptr, nullptr
		);
		return;
	}
	/* Respond */
	cColor color = get_lmg_member_color(*member);
	if (db_result.empty()) {
		/* User not registered in the leaderboard */
		EditInteractionResponse(
			interaction, nullptr, MESSAGE_FLAG_NONE,
			std::vector<cEmbed> { make_no_xp_embed(*user, color) },
			nullptr, nullptr, nullptr
		);
	}
	else {
		/* Respond to interaction with a proper embed */
		auto &res = db_result[0];
		EditInteractionResponse(
			interaction, nullptr, MESSAGE_FLAG_NONE,
			std::vector<cEmbed> {
				make_embed(*user, *member, color, res.GetRank(), res.GetXp(), res.GetNumMessages())
			}, nullptr,
			std::vector<cActionRow>{
				cActionRow{
					cButton<BUTTON_STYLE_SECONDARY>{
						STR(CMP_ID_BUTTON_RANK_HELP),
						"How does this work?"
					}
				}
			}, nullptr
		);
	}
}

void
cGreekBot::OnInteraction_top(chInteraction interaction) {
	try {
		/* Acknowledge interaction */
		AcknowledgeInteraction(interaction);
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
		/* Prepare embeds */
		std::vector<cEmbed> embeds;
		embeds.reserve(db_result.size());
		for (auto &d: db_result) {
			try {
				/* Get member info and create embeds */
				if (*d.GetUserId() == interaction->GetMember()->GetUser()->GetId()) {
					chMember member = interaction->GetMember();
					embeds.push_back(make_embed(*member->GetUser(), *member, get_lmg_member_color(*member), d.GetRank(), d.GetXp(), d.GetNumMessages()));
				}
				else {
					cMember member = GetGuildMember(m_lmg_id, *d.GetUserId());
					embeds.push_back(make_embed(*member.GetUser(), member, get_lmg_member_color(member), d.GetRank(), d.GetXp(), d.GetNumMessages()));
				}
			}
			catch (const xDiscordError& e) {
				/* User isn't a member anymore */
				embeds.push_back(make_no_member_embed(GetUser(*d.GetUserId()), true));
			}
		}
		/* Respond to interaction */
		EditInteractionResponse(
			interaction, nullptr, MESSAGE_FLAG_NONE, embeds, nullptr,
			std::vector<cActionRow>{
				cActionRow{
					cButton<BUTTON_STYLE_SECONDARY>{
						STR(CMP_ID_BUTTON_RANK_HELP),
						"How does this work?"
					}
				}
			}, nullptr
		);
	}
	catch (const std::exception& e) {
		cUtils::PrintErr("OnInteraction_top: %s", e.what());
	}
}