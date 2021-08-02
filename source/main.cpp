#include "Bot.h"
#include "Discord.h"
#define TOKEN "ODYzNDk5MTUxMTgxMjE3Nzky.YOnyQA.6XWkgM_P4S4lE-t8jhT_dD2qQlk"

class cGreekBot final : public cBot {
private:
	
	void OnInteraction_avatar(chInteraction interaction) {
		if (interaction->GetData()->Options.empty()) {
			// TODO: avatar of sender
			return;
		}
		
		auto opt = interaction->GetData()->Options[0];
		if (0 == strcmp(opt->GetName(), "user")) {
			if (opt->GetType() == APP_COMMAND_USER) {
				auto user = interaction->GetData()->GetResolvedData()->Users[opt->GetValue<APP_COMMAND_USER>()];
				char d[256];
				// TODO: response class
				sprintf(d, "{\"type\":4,\"data\":{\"content\":\"%s\"}}", user->GetAvatarUrl());
				cDiscord::RespondToInteraction(m_http_auth, interaction->GetId()->ToString(), interaction->GetToken(), d);
			}
		}
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
	cGreekBot() : cBot(TOKEN) {}
	
};

int main() {
	cGreekBot().Run();
	return 0;
}
