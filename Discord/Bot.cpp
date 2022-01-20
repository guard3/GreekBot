#include "Bot.h"
#include "Utils.h"
#include "Discord.h"
#include "Net.h"

void cBot::OnReady(uchUser user) {
	cUtils::PrintLog("Connected as: %s#%s %s", user->GetUsername(), user->GetDiscriminator(), user->GetId()->ToString());
	delete m_user;
	m_user = user.release();
}

void cBot::respond_to_interaction(chInteraction interaction, json::object &&obj) {
	char path[300];
	sprintf(path, "%s/interactions/%s/%s/callback", DISCORD_API_ENDPOINT, interaction->GetId()->ToString(), interaction->GetToken());
	cNet::PostHttpsRequest(DISCORD_API_HOST, path, GetHttpAuthorization(), obj);
}

void cBot::edit_interaction_response(chInteraction interaction, json::object &&obj) {
	char path[512];
	sprintf(path, "%s/webhooks/%s/%s/messages/@original", DISCORD_API_ENDPOINT, interaction->GetApplicationId()->ToString(), interaction->GetToken());
	cNet::PatchHttpsRequest(DISCORD_API_HOST, path, GetHttpAuthorization(), obj.at("data").as_object());
}

void cBot::AddGuildMemberRole(const cSnowflake& guild_id, const cSnowflake& user_id, const cSnowflake &role_id) {
	char path[300];
	sprintf(path, "%s/guilds/%s/members/%s/roles/%s", DISCORD_API_ENDPOINT, guild_id.ToString(), user_id.ToString(), role_id.ToString());
	cNet::PutHttpsRequest(DISCORD_API_HOST, path, GetHttpAuthorization());
}

void cBot::RemoveGuildMemberRole(const cSnowflake& guild_id, const cSnowflake& user_id, const cSnowflake &role_id) {
	char path[300];
	sprintf(path, "%s/guilds/%s/members/%s/roles/%s", DISCORD_API_ENDPOINT, guild_id.ToString(), user_id.ToString(), role_id.ToString());
	cNet::DeleteHttpsRequest(DISCORD_API_HOST, path, GetHttpAuthorization());
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
	cNet::PatchHttpsRequest(DISCORD_API_HOST, path, GetHttpAuthorization(), obj);
}