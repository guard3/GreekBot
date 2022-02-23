#include "GreekBot.h"
#include "Database.h"

/* Resolve level info from XP */
struct sResolvedLevel {
	int64_t level;   // Current level
	int64_t base_xp; // Current level's base XP
	int64_t next_xp; // Next level's base XP
};
static sResolvedLevel xp_to_level(int64_t xp) {
	sResolvedLevel result { 0, 0, 100 };
	while (result.next_xp < xp) {
		result.level++;
		result.base_xp = result.next_xp;
		result.next_xp += 5 * result.level * result.level + 50 * result.level + 100;
	}
	return result;
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
			std::vector<cEmbed> {
				cEmbed::CreateBuilder()
					.SetAuthor(cUtils::Format("%s#%s", user->GetUsername(), user->GetDiscriminator()).c_str(), nullptr, user->GetAvatarUrl())
					.SetDescription(cUtils::Format("User is not a member of **Learning Greek**%s", db_result.empty() ? "." : " anymore.").c_str())
					.SetColor(0x0096FF)
					.Build()
			}, nullptr, nullptr, nullptr
		);
		return;
	}
	/* Sort guild roles based on position */
	m_lmg.mutex.lock();
	if (m_lmg.sorted_roles.empty()) {
		for (auto& r : m_lmg.roles)
			m_lmg.sorted_roles.push_back(&r);
		std::sort(m_lmg.sorted_roles.begin(), m_lmg.sorted_roles.end(), [](chRole a, chRole b) { return a->GetPosition() > b->GetPosition(); });
	}
	/* Calculate member color */
	cColor color;
	for (auto& r : m_lmg.sorted_roles) {
		if (std::find_if(member->Roles.begin(), member->Roles.end(), [&r](const chSnowflake& a) { return *r->GetId() == *a;}) != member->Roles.end()) {
			if (r->GetColor()) {
				color = r->GetColor();
				break;
			}
		}
	}
	m_lmg.mutex.unlock();
	cEmbedBuilder builder = cEmbed::CreateBuilder().SetColor(color).SetAuthor(cUtils::Format("%s#%s", user->GetUsername(), user->GetDiscriminator()).c_str(), nullptr, user->GetAvatarUrl());
	if (db_result.empty()) {
		/* User not registered in the leaderboard */
		EditInteractionResponse(
			interaction, nullptr, MESSAGE_FLAG_NONE,
			std::vector<cEmbed> { builder.SetDescription("User has no XP yet").Build() },
			nullptr, nullptr, nullptr
		);
		return;
	}
	/* Calculate level */
	auto& res = db_result[0];
	sResolvedLevel lvl = xp_to_level(res.GetXp());
	/* Respond to interaction */
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
			builder
				.SetTitle(cUtils::Format("%s Rank **#%" PRIi64 "**    Level **%" PRIi64 "**", medal, res.GetRank(), lvl.level).c_str())
				.AddField("XP Progress", cUtils::Format("%" PRIi64 "/%" PRIi64, res.GetXp() - lvl.base_xp, lvl.next_xp - lvl.base_xp).c_str(), true)
				.AddField("Total XP", std::to_string(res.GetXp()).c_str(), true)
				.AddField("Messages", std::to_string(res.GetNumMessages()).c_str(), true)
				.Build()
		}, nullptr,
		std::vector<cActionRow> {
			cActionRow {
				cButton<BUTTON_STYLE_SECONDARY> {
					STR(CMP_ID_BUTTON_RANK_HELP),
					"How does this work?"
				}
			}
		}, nullptr
	);
}

void
cGreekBot::OnInteraction_top(chInteraction interaction) {
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
	/* Sort guild roles based on position */
	m_lmg.mutex.lock();
	if (m_lmg.sorted_roles.empty()) {
		for (auto& r : m_lmg.roles)
			m_lmg.sorted_roles.push_back(&r);
		std::sort(m_lmg.sorted_roles.begin(), m_lmg.sorted_roles.end(), [](chRole a, chRole b) { return a->GetPosition() > b->GetPosition(); });
	}
	/* Display data */
	std::string str;
	std::vector<cEmbed> embeds;
	embeds.reserve(db_result.size());
	uchMember u_member;
	uchUser   u_user;
	chMember  member;
	chUser    user;
	for (auto &d: db_result) {
		if (*d.GetUserId() == *interaction->GetMember()->GetUser()->GetId()) {
			member = interaction->GetMember();
			user = member->GetUser();
		}
		else if ((u_member = GetGuildMember(m_lmg.guild->GetId(), d.GetUserId()))) {
			/* Try to resolve member object */
			member = u_member.get();
			user = member->GetUser();
		}
		else {
			/* If user isn't a member anymore... TBA */
		}

		cColor color;
		for (auto& r : m_lmg.sorted_roles) {
			if (std::find_if(member->Roles.begin(), member->Roles.end(), [&r](const chSnowflake& a) { return *r->GetId() == *a;}) != member->Roles.end()) {
				if (r->GetColor()) {
					color = r->GetColor();
					break;
				}
			}
		}

		sResolvedLevel lvl = xp_to_level(d.GetXp());
		const char* medal;
		switch (d.GetRank()) {
			case 1:  medal = "🥇"; break;
			case 2:  medal = "🥈"; break;
			case 3:  medal = "🥉"; break;
			default: medal = "🏅"; break;
		}
		embeds.emplace_back(
			cEmbed::CreateBuilder()
				.SetAuthor(cUtils::Format("%s#%s", user->GetUsername(), user->GetDiscriminator()).c_str(), nullptr, user->GetAvatarUrl())
				.SetTitle(cUtils::Format("%s Rank **#%" PRIi64 "**\tLevel **%" PRIi64 "**", medal, d.GetRank(), lvl.level).c_str())
				.SetColor(color)
				.AddField("XP Progress", cUtils::Format("%" PRIi64 "/%" PRIi64, d.GetXp() - lvl.base_xp, lvl.next_xp - lvl.base_xp).c_str(), true)
				.AddField("Total XP", std::to_string(d.GetXp()).c_str(), true)
				.AddField("Messages", std::to_string(d.GetNumMessages()).c_str(), true)
				.Build()
		);
	}
	m_lmg.mutex.unlock();
	EditInteractionResponse(
		interaction, str.c_str(), MESSAGE_FLAG_NONE, embeds, nullptr,
		std::vector<cActionRow> {
			cActionRow {
				cButton<BUTTON_STYLE_SECONDARY> {
					STR(CMP_ID_BUTTON_RANK_HELP),
					"How does this work?"
				}
			}
		}, nullptr
	);
}
