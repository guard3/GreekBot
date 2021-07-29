#include "Bot.h"
#include "Gateway.h"
#include "Utils.h"
#include "Discord.h"

cBot::cBot(const char* token) {
	/* Add bot token to the http auth string */
	if (token) {
		strncpy(m_token, token, 59);
		m_token[59] = '\0';
	}
}

void cBot::OnInteractionCreate(uchInteraction interaction) {
	cUtils::PrintLog("Command ID: %s", interaction->GetData()->GetCommandId()->ToString());
	
	if (interaction->GetData()->GetCommandId()->ToInt() == 869284421042307073) {
		cDiscord::RespondToInteraction(m_http_auth, interaction->GetId()->ToString(), interaction->GetToken(), "{\"type\":4,\"data\":{\"content\":\"BONK!!!\"}}");
	}
}

void cBot::Run() {
	cGateway gateway(m_token);
	gateway.SetOnReady([this](uchUser user) {
		cUtils::PrintLog("Connected as: %s#%d %s", user->GetUsername(), user->GetDiscriminator(), user->GetId()->ToString());
		m_user = std::move(user);
	}).SetOnInteractionCreate([this](uchInteraction interaction) {
		OnInteractionCreate(std::move(interaction));
	}).Run();
}
