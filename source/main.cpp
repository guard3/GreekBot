#include "Bot.h"
#include "Discord.h"
#include "Utils.h"
#define TOKEN "ODYzNDk5MTUxMTgxMjE3Nzky.YOnyQA.6XWkgM_P4S4lE-t8jhT_dD2qQlk"

class cGreekBot final : public cBot {
private:
	
	void OnInteraction_avatar(chInteraction interaction) {
		char avatar_url[200];
		if (interaction->GetData()->Options.empty()) {
			/* If no option is provided, use sender's avatar */
			strcpy(avatar_url, interaction->GetMember()->GetUser()->GetAvatarUrl());
		}
		else {
			/* Otherwise, get "user" option avatar */
			auto opt = interaction->GetData()->Options[0];
			if (0 == strcmp(opt->GetName(), "user")) {
				if (opt->GetType() == APP_COMMAND_USER) {
					auto user_id = opt->GetValue<APP_COMMAND_USER>();
					auto user = interaction->GetData()->GetResolvedData()->Users[user_id];
					strcpy(avatar_url, user->GetAvatarUrl());
				}
				else return;
			}
			else return;
		}
		
		char d[256];
		// TODO: response class
		sprintf(d, "{\"type\":4,\"data\":{\"content\":\"%s\"}}", avatar_url);
		cDiscord::RespondToInteraction(m_http_auth, interaction->GetId()->ToString(), interaction->GetToken(), d);
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
