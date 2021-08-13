#include "Bot.h"
#include "Gateway.h"
#include "Utils.h"
#include "Discord.h"
#include "Net.h"
#include <iostream>
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

void cBot::AddGuildMemberRole(const cSnowflake& guild_id, const cSnowflake& user_id, const cSnowflake &role_id) {
	char path[300];
	sprintf(path, "%s/guilds/%s/members/%s/roles/%s", DISCORD_API_ENDPOINT, guild_id.ToString(), user_id.ToString(), role_id.ToString());
	cNet::PutHttpsRequest(DISCORD_API_HOST, path, m_http_auth);
}

void cBot::RemoveGuildMemberRole(const cSnowflake& guild_id, const cSnowflake& user_id, const cSnowflake &role_id) {
	char path[300];
	sprintf(path, "%s/guilds/%s/members/%s/roles/%s", DISCORD_API_ENDPOINT, guild_id.ToString(), user_id.ToString(), role_id.ToString());
	cNet::DeleteHttpsRequest(DISCORD_API_HOST, path, m_http_auth);
}

void cBot::UpdateGuildMemberRoles(const cSnowflake& guild_id, const cSnowflake& user_id, const std::vector<chSnowflake>& role_ids) {
	/* Prepare json response */
	json::object obj;
	{
		json::array a;
		a.reserve(role_ids.size());
		for (chSnowflake s : role_ids)
			a.push_back(s->ToString());
		obj["roles"] = std::move(a);
	}
	/* Resolve api path */
	char path[80];
	sprintf(path, "%s/guilds/%s/members/%s", DISCORD_API_ENDPOINT, guild_id.ToString(), user_id.ToString());
	/* Perform http request */
	cNet::PatchHttpsRequest(DISCORD_API_HOST, path, m_http_auth, obj);
}

void cBot::RespondToInteraction(const char *interaction_id, const char *token, const std::string &data) {
	cDiscord::RespondToInteraction(m_http_auth, interaction_id, token, data);
}

void cBot::EditInteractionResponse(chInteraction interaction, const cInteractionResponse<INTERACTION_CALLBACK_UPDATE_MESSAGE> &response) {
	char path[512];
	sprintf(path, "%s/webhooks/%s/%s/messages/@original", DISCORD_API_ENDPOINT, interaction->GetApplicationId()->ToString(), interaction->GetToken());
	cNet::PatchHttpsRequest(DISCORD_API_HOST, path, m_http_auth, response.ToJson().at("data").as_object());
}
