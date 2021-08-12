#include "Bot.h"
#include "Component.h"
#include <iostream>
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
			"Test <:mlem:594150641740414987>",
			cActionRow {
				cSelectMenu {
					"meow",
					"Dunno, select smth",
					cSelectOption {
						"Option1",
						"opt1",
						"This is an option"
					},
					cSelectOption {
						"Option2",
						"opt2",
						"This is another option"
					}
				}
			},
			cActionRow {
				cButton<BUTTON_STYLE_PRIMARY> {
					"butt1",
					"Hello"
				},
				cButton<BUTTON_STYLE_SECONDARY> {
					"butt2",
					"World"
				},
				cButton<BUTTON_STYLE_SUCCESS> {
					"butt3",
					"Mlem"
				},
				cButton<BUTTON_STYLE_DANGER> {
					"butt4",
					"Mlom"
				},
				cButton<BUTTON_STYLE_LINK> {
					"https://youtu.be/dQw4w9WgXcQ", // LOL
					"Don't click this!"
				}
			}
		};
		std::cout << r.ToJsonString() << std::endl;
		RespondToInteraction(interaction, r);
	}
	
	void OnInteractionCreate(chInteraction interaction) override {
		switch (interaction->GetData()->GetCommandId()->ToInt()){
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
	
public:
	cGreekBot(const char* token) : cBot(token) {}
};

int main(int argc, const char** argv) {
	cGreekBot(argv[1]).Run();
	return 0;
}
