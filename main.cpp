#include "Bot.h"
#include "Component.h"
#include "Utils.h"
#include "Database.h"
#include "Embed.h"
class cGreekBot final : public cBot {
private:
	enum eLmgProficiencyRoleId {
		LMG_PROFICIENCY_NATIVE,
		LMG_PROFICIENCY_BEGINNER,
		LMG_PROFICIENCY_ELEMENTARY,
		LMG_PROFICIENCY_INTERMEDIATE,
		LMG_PROFICIENCY_UPPER_INTERMEDIATE,
		LMG_PROFICIENCY_ADVANCED,
		LMG_PROFICIENCY_FLUENT,
		LMG_PROFICIENCY_NON_LEARNER,
		LMG_NUM_PROFICIENCY_ROLES
	};
	cSnowflake m_lmg_id = 350234668680871946; // Learning Greek
	cSnowflake m_lmg_proficiency_roles[8] {
		350483752490631181, // @Native
		351117824300679169, // @Beginner
		351117954974482435, // @Elementary
		350485376109903882, // @Intermediate
		351118486426091521, // @Upper Intermediate
		350485279238258689, // @Advanced
		350483489461895168, // @Fluent
		352001527780474881  // @Non Learner
	};

	/* Voice */
	std::vector<uint64_t> m_lmg_voice_channels;
	std::vector<std::vector<uint64_t>> m_lmg_users_connected_to_voice;

	void OnInteraction_avatar(chInteraction interaction) {
		auto data = interaction->GetData<INTERACTION_APPLICATION_COMMAND>();

		chUser user;
		if (data->Options.empty()) {
			/* If no option is provided, use sender's avatar */
			user = interaction->GetUser() ? interaction->GetUser() : interaction->GetMember()->GetUser();
		}
		else {
			/* Otherwise, get "user" option avatar */
			auto option = data->Options[0];
			if (!(user = option->GetValue<APP_COMMAND_OPT_USER>()))
				return;
		}

		RespondToInteraction(interaction, user->GetAvatarUrl(), MESSAGE_FLAG_NONE, nullptr, nullptr, nullptr, nullptr);
	}

	void OnInteraction_role(chInteraction interaction) {
		RespondToInteraction(
			interaction,
			"Select a role depending on your greek level:",
			MESSAGE_FLAG_EPHEMERAL,
			nullptr,
			nullptr,
			std::vector<cActionRow>{
				cActionRow{
					cSelectMenu{
						"proficiency_role_menu",
						"Choose an option...",
						cSelectOption{
							"Native",
							"opt_gr",
							"If greek is your native language",
							cEmoji("level_gr", "875469185529036880")
						},
						cSelectOption{
							"Beginner",
							"opt_a1",
							"If you just started learning greek or your level is A1",
							cEmoji("level_a1", "875469185793286164")
						},
						cSelectOption{
							"Elementary",
							"opt_a2",
							"If your greek level is A2",
							cEmoji("level_a2", "875469185394827355")
						},
						cSelectOption{
							"Intermediate",
							"opt_b1",
							"If your greek level is B1",
							cEmoji("level_b1", "875469185659056138")
						},
						cSelectOption{
							"Upper Intermediate",
							"opt_b2",
							"If your greek level is B2",
							cEmoji("level_b2", "875469185751347251")
						},
						cSelectOption{
							"Advanced",
							"opt_c1",
							"If your greek level is C1",
							cEmoji("level_c1", "875469185726173276")
						},
						cSelectOption{
							"Fluent",
							"opt_c2",
							"If your greek level is C2",
							cEmoji("level_c2", "875469185734541382")
						},
						cSelectOption{
							"Non Learner",
							"opt_no",
							"If you don't want to learn greek",
							cEmoji("level_no", "875469185466109992")
						}
					}
				},
				cActionRow{
					cButton<BUTTON_STYLE_LINK>{
						"https://en.wikipedia.org/wiki/Common_European_Framework_of_Reference_for_Languages",
						"Don't know what to pick?"
					}
				}
			},
			nullptr
		);
	}

	void OnInteraction_connect(chInteraction interaction) {
		chMember member = interaction->GetMember();
		//member->

	}

	void lmg_update_proficiency_role(chMember member, eLmgProficiencyRoleId proficiency_role) {
		/* The new roles for the member */
		std::vector<chSnowflake> roles;
		roles.reserve(member->Roles.size());
		/* Copy all existing roles except proficiency roles */
		cSnowflake *proficiency_roles_begin = m_lmg_proficiency_roles, *proficiency_roles_end = m_lmg_proficiency_roles + LMG_NUM_PROFICIENCY_ROLES;
		for (chSnowflake p : member->Roles) {
			if (proficiency_roles_end == std::find_if(proficiency_roles_begin, proficiency_roles_end, [&](const cSnowflake& s) { return s.ToInt() == p->ToInt(); }))
				roles.push_back(p);
		}
		/* Add specified proficiency role */
		roles.push_back(&m_lmg_proficiency_roles[proficiency_role]);
		UpdateGuildMemberRoles(m_lmg_id, *member->GetUser()->GetId(), roles);
	}

	void OnInteraction_MessageComponent(chInteraction interaction) {
		/* Acknowledge interaction */
		AcknowledgeInteraction(interaction);
		const char* value = interaction->GetData<INTERACTION_MESSAGE_COMPONENT>()->Values[0];
		auto member = interaction->GetMember();

		if (0 == strcmp(value, "opt_gr")) {
			lmg_update_proficiency_role(member, LMG_PROFICIENCY_NATIVE);
		}
		else if (0 == strcmp(value, "opt_a1")) {
			lmg_update_proficiency_role(member, LMG_PROFICIENCY_BEGINNER);
		}
		else if (0 == strcmp(value, "opt_a2")) {
			lmg_update_proficiency_role(member, LMG_PROFICIENCY_ELEMENTARY);
		}
		else if (0 == strcmp(value, "opt_b1")) {
			lmg_update_proficiency_role(member, LMG_PROFICIENCY_INTERMEDIATE);
		}
		else if (0 == strcmp(value, "opt_b2")) {
			lmg_update_proficiency_role(member, LMG_PROFICIENCY_UPPER_INTERMEDIATE);
		}
		else if (0 == strcmp(value, "opt_c1")) {
			lmg_update_proficiency_role(member, LMG_PROFICIENCY_ADVANCED);
		}
		else if (0 == strcmp(value, "opt_c2")) {
			lmg_update_proficiency_role(member, LMG_PROFICIENCY_FLUENT);
		}
		else if (0 == strcmp(value, "opt_no")) {
			lmg_update_proficiency_role(member, LMG_PROFICIENCY_NON_LEARNER);
		}
		/* Edit original interaction message */
		EditInteractionResponse(interaction, "Role assigned!", MESSAGE_FLAG_EPHEMERAL, nullptr, nullptr, std::vector<cActionRow>(), nullptr);
	}

	void OnGuildCreate(chGuild guild) override {
		if (guild->GetId()->ToInt() == m_lmg_id.ToInt()) {
			cUtils::PrintLog("HELLO LMG!!!");
			for (auto& channel : guild->Channels) {
				if (channel->GetType() == CHANNEL_GUILD_VOICE)
					m_lmg_voice_channels.push_back(channel->GetId()->ToInt());
			}
			for (auto& voice_state : guild->VoiceStates) {
				auto it = std::find(m_lmg_voice_channels.begin(), m_lmg_voice_channels.end(), voice_state->GetChannelId()->ToInt());
				if (it != std::end(m_lmg_voice_channels)) {

				}
			}
		}
	}

	void OnInteraction_rank(chInteraction interaction) {
		/* Retrieve user argument */
		auto data = interaction->GetData<INTERACTION_APPLICATION_COMMAND>();
		chUser user = data->Options.empty() ? interaction->GetMember()->GetUser() : data->Options[0]->GetValue<APP_COMMAND_OPT_USER>();
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
			EditInteractionResponse(interaction, "Hmm... Looks like I've run into some trouble. Try again later!", MESSAGE_FLAG_NONE, nullptr, nullptr, nullptr, nullptr);
			return;
		}
		/* Calculate level */
		if (!db_result.empty()) {
			auto& res = db_result[0];
			for (uint64_t curr_lvl = 0, curr_xp = 0, next_lvl = 1, next_xp;;) {
				next_xp = curr_xp + 5 * (next_lvl * next_lvl) + 40 * (next_lvl) + 55;
				if (next_xp > db_result[0].GetXp()) {
					/* Prepare embed author name */
					char embed_author[128];
					sprintf(embed_author, "%s#%s", user->GetUsername(), user->GetDiscriminator());
					/* Prepare embed title */
					char embed_title[128];
					const char* medal;
					switch (res.GetRank()) {
						case 1:
							medal = "🥇";
							break;
						case 2:
							medal = "🥈";
							break;
						case 3:
							medal = "🥉";
							break;
						default:
							medal = "🏅";
					}
					sprintf(embed_title, "%s Rank **#%d**", medal, (int)res.GetRank());
					EditInteractionResponse(
						interaction,
						nullptr,//str,
						MESSAGE_FLAG_NONE,
						std::vector<cEmbed> {
							cEmbed::CreateBuilder()
							.SetAuthor(embed_author, nullptr, user->GetAvatarUrl())
							.SetTitle(embed_title)
							.SetColor(0x5bc2e9) // TODO: Get user's role color
							.AddField("Level", std::to_string(curr_lvl).c_str(), true)
							.AddField("XP Progress", (std::to_string(next_xp - res.GetXp()) + '/' + std::to_string(next_xp - curr_xp)).c_str(), true)
							.AddField("Total XP", std::to_string(res.GetXp()).c_str(), true)
							.SetFooter("Keep talking and establish yourself in the leaderboard!", nullptr)
							.Build()
						},
						nullptr,
						nullptr,
						nullptr
					);
					// TODO: Maybe add a button that will explain how the ranking system works
					return;
				}
				curr_xp = next_xp;
				curr_lvl++;
				next_lvl++;
			}
		}
		EditInteractionResponse(interaction, "User has no XP yet!", MESSAGE_FLAG_NONE, nullptr, nullptr, nullptr, nullptr);
	}

	void OnInteraction_top(chInteraction interaction) {
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
		/* Display data */
		// TODO: make this pretty with embeds and stuff...
		std::string str;
		for (auto &d: db_result) {
			char test[500];
			sprintf(test, "Rank #%d, User id: %s", (int)d.GetRank(), d.GetUserId()->ToString());
			str += test;
			str += '\n';
		}
		EditInteractionResponse(interaction, str.c_str(), MESSAGE_FLAG_NONE, nullptr, nullptr, nullptr, nullptr);
	}

	void OnInteractionCreate(chInteraction interaction) override {
		if (interaction->GetType() == INTERACTION_APPLICATION_COMMAND) {
			switch (interaction->GetData<INTERACTION_APPLICATION_COMMAND>()->GetCommandId()->ToInt()) {
				case 878391425568473098:
					/* avatar */
					OnInteraction_avatar(interaction);
					break;
				case 874634186374414356:
					/* role */
					OnInteraction_role(interaction);
					break;
				case 938199801420456066:
					/* rank */
					OnInteraction_rank(interaction);
					break;
				case 938863857466757131:
					/* top */
					OnInteraction_top(interaction);
					break;
				case 904462004071313448:
					/* connect */
					OnInteraction_connect(interaction);
					break;
				default:
					break;
			}
		}
		else if (interaction->GetType() == INTERACTION_MESSAGE_COMPONENT)
			OnInteraction_MessageComponent(interaction);
	}

	void OnMessageCreate(chMessage msg) override {
		/* Ignore messages of bots and system users */
		if (chUser author = msg->GetAuthor(); author->IsBotUser() || author->IsSystemUser())
			return;
		/* Update leaderboard */
		cDatabase::UpdateLeaderboard(msg);
	}
	
public:
	explicit cGreekBot(const char* token) : cBot(token, INTENT_GUILD_INTEGRATIONS | INTENT_GUILD_MESSAGES) {}
};

int main(int argc, const char** argv) {
	cGreekBot(argv[1]).Run();
	return 0;
}
