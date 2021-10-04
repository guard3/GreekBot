#include "Bot.h"
#include "Component.h"
#include "Utils.h"
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

		cInteractionResponse<INTERACTION_CALLBACK_CHANNEL_MESSAGE_WITH_SOURCE> r(user->GetAvatarUrl());
		RespondToInteraction(interaction, r);
	}

	void OnInteraction_role(chInteraction interaction) {
		RespondToInteraction<INTERACTION_CALLBACK_CHANNEL_MESSAGE_WITH_SOURCE>(interaction, "Select a role depending on your greek level:", INTERACTION_FLAG_EPHEMERAL, cActionRow {
				cSelectMenu {
					"proficiency_role_menu",
					"Choose an option...",
					cSelectOption {
						"Native",
						"opt_gr",
						"If greek is your native language",
						cEmoji("level_gr", "875469185529036880")
					},
					cSelectOption {
						"Beginner",
						"opt_a1",
						"If you just started learning greek or your level is A1",
						cEmoji("level_a1", "875469185793286164")
					},
					cSelectOption {
						"Elementary",
						"opt_a2",
						"If your greek level is A2",
						cEmoji("level_a2", "875469185394827355")
					},
					cSelectOption {
						"Intermediate",
						"opt_b1",
						"If your greek level is B1",
						cEmoji("level_b1", "875469185659056138")
					},
					cSelectOption {
						"Upper Intermediate",
						"opt_b2",
						"If your greek level is B2",
						cEmoji("level_b2", "875469185751347251")
					},
					cSelectOption {
						"Advanced",
						"opt_c1",
						"If your greek level is C1",
						cEmoji("level_c1", "875469185726173276")
					},
					cSelectOption {
						"Fluent",
						"opt_c2",
						"If your greek level is C2",
						cEmoji("level_c2", "875469185734541382")
					},
					cSelectOption {
						"Non Learner",
						"opt_no",
						"If you don't want to learn greek",
						cEmoji("level_no", "875469185466109992")
					}
				}
			},
			cActionRow {
				cButton<BUTTON_STYLE_LINK> {
					"https://en.wikipedia.org/wiki/Common_European_Framework_of_Reference_for_Languages",
					"Don't know what to pick?"
				}
			}
		);
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
		RespondToInteraction<INTERACTION_CALLBACK_DEFERRED_UPDATE_MESSAGE>(interaction);
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
		EditInteractionResponse(interaction, "Role assigned!", INTERACTION_FLAG_EPHEMERAL | INTERACTION_FLAG_REMOVE_COMPONENTS);
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
				default:
					break;
			}
		}
		else if (interaction->GetType() == INTERACTION_MESSAGE_COMPONENT)
			OnInteraction_MessageComponent(interaction);
	}
	
public:
	cGreekBot(const char* token) : cBot(token) {}
};

int main(int argc, const char** argv) {
	cGreekBot(argv[1]).Run();
	return 0;
}
