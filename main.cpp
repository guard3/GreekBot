#include "Bot.h"
#include "Component.h"
#include <iostream>
#include "Utils.h"
class cGreekBot final : public cBot {
private:
	void OnInteraction_avatar(chInteraction interaction) {
		auto data = interaction->GetData();

		chUser user;
		if (data->Options.empty()) {
			/* If no option is provided, use sender's avatar */
			user = interaction->GetUser() ? interaction->GetUser() : interaction->GetMember()->GetUser();
		}
		else {
			/* Otherwise, get "user" option avatar */
			auto option = data->Options[0];
			if (0 != strcmp(option->GetName(), "user")) return;
			if (option->GetType() != APP_COMMAND_USER) return;
			user = option->GetValue<APP_COMMAND_USER>();
		}

		cInteractionResponse<INTERACTION_CALLBACK_CHANNEL_MESSAGE_WITH_SOURCE> r(user->GetAvatarUrl());
		RespondToInteraction(interaction, r);
	}

	void OnInteraction_role(chInteraction interaction) {
		cInteractionResponse<INTERACTION_CALLBACK_CHANNEL_MESSAGE_WITH_SOURCE> r {
			"Select a role depending on your greek level:",
			INTERACTION_FLAG_EPHEMERAL,
			cActionRow {
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
		};
		std::cout << r.ToJsonString() << std::endl;
		RespondToInteraction(interaction, r);
	}

	void OnInteraction_MessageComponent(chInteraction interaction) {
		const char* value = interaction->GetData()->Values[0];
		if (0 == strcmp(value, "opt_gr")) {
			cUtils::PrintLog("NATIVE");
		}
		else if (0 == strcmp(value, "opt_a1")) {
			cUtils::PrintLog("BEGINNER");
		}
		else if (0 == strcmp(value, "opt_a2")) {
			cUtils::PrintLog("ELEMENTARY");
		}
		else if (0 == strcmp(value, "opt_b1")) {
			cUtils::PrintLog("INTERMEDIATE");
		}
		else if (0 == strcmp(value, "opt_b2")) {
			cUtils::PrintLog("UPPER INTERMEDIATE");
		}
		else if (0 == strcmp(value, "opt_c1")) {
			cUtils::PrintLog("ADVANCED");
		}
		else if (0 == strcmp(value, "opt_c2")) {
			cUtils::PrintLog("FLUENT");
		}
		else if (0 == strcmp(value, "opt_no")) {
			cUtils::PrintLog("NON LEARNER");
		}
		cInteractionResponse<INTERACTION_CALLBACK_UPDATE_MESSAGE> r("MEOW", INTERACTION_FLAG_EPHEMERAL);
		std::cout << r.ToJson() << std::endl;
		RespondToInteraction(interaction, r);
	}
	
	void OnInteractionCreate(chInteraction interaction) override {
		if (interaction->GetType() == INTERACTION_APPLICATION_COMMAND) {
			switch (interaction->GetData()->GetCommandId()->ToInt()) {
				case 870286903545589840:
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
