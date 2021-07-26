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

bool cBot::RegisterSlashCommand(const char *name, const char *description) {
	return true;
}

void cBot::Run() {
	cGateway gateway(m_token);
	gateway.SetOnReady([this](hUser user) {
		const char* application_id = user->GetId();
		if (!m_user)
			cDiscord::RegisterSlashCommand(m_http_auth, application_id, { "bonk", "bink" });
		m_user = user;
		cUtils::PrintLog("%s#%s", m_user->GetUsername(), m_user->GetDiscriminator());
	}).Run();
}
