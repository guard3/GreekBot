#include "Bot.h"
#include "Gateway.h"
#include "Utils.h"
#include "Discord.h"

void cBot::RespondToInteraction(const char *interaction_id, const char *token, const std::string &data) {
	cDiscord::RespondToInteraction(m_http_auth, interaction_id, token, data);
}

cBot::cBot(const char* token) {
	/* Add bot token to the http auth string */
	if (token) {
		strncpy(m_token, token, 59);
		m_token[59] = '\0';
	}
}

void cBot::Run() {
	cGateway gateway(m_token);
	gateway.SetOnReady([this](uchUser user) {
		cUtils::PrintLog("Connected as: %s#%s %s", user->GetUsername(), user->GetDiscriminator(), user->GetId()->ToString());
		m_user = std::move(user);
	}).SetOnInteractionCreate([this](chInteraction interaction) {
		OnInteractionCreate(interaction);
	}).Run();
}
