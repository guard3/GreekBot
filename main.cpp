#include "Bot.h"
#include "Discord.h"
#include "Utils.h"
#include "InteractionResponse.h"

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
	
	void OnInteractionCreate(chInteraction interaction) override {
		switch (interaction->GetData()->GetCommandId()->ToInt()){
			case 870286903545589840:
				/* avatar */
				OnInteraction_avatar(interaction);
				
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
