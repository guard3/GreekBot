#include "Bot.h"
#include "Utils.h"
#include "Discord.h"
#include "Net.h"

void cBot::OnReady(uchUser user) {
	cUtils::PrintLog("Connected as: %s#%s %s", user->GetUsername(), user->GetDiscriminator(), user->GetId()->ToString());
	delete m_user;
	m_user = user.release();
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

/* Interaction related functions */

enum eInteractionCallbackType {
	INTERACTION_CALLBACK_PONG                                 = 1, // ACK a Ping
	INTERACTION_CALLBACK_CHANNEL_MESSAGE_WITH_SOURCE          = 4, // respond to an interaction with a message
	INTERACTION_CALLBACK_DEFERRED_CHANNEL_MESSAGE_WITH_SOURCE = 5, // ACK an interaction and edit a response later, the user sees a loading state
	INTERACTION_CALLBACK_DEFERRED_UPDATE_MESSAGE              = 6, // for components, ACK an interaction and edit the original message later; the user does not see a loading state
	INTERACTION_CALLBACK_UPDATE_MESSAGE                       = 7  // for components, edit the message the component was attached to
	// 8 TODO: implement autocomplete result interaction stuff... someday
};

json::object cBot::sIF_data::to_json() const {
	json::object obj;
	if (content)
		obj["content"] = content;
	obj["tts"] = (bool)(flags & MESSAGE_FLAG_TTS);
	obj["flags"] = flags & (MESSAGE_FLAG_EPHEMERAL | MESSAGE_FLAG_SUPPRESS_EMBEDS);
	if (num_embeds != -1) {
		json::array a;
		a.reserve(num_embeds);
		for (int32_t i = 0; i < num_embeds; ++i)
			a.push_back(embeds[i].ToJson());
		obj["embeds"] = std::move(a);
	}
	if (num_components != -1) {
		json::array a;
		a.reserve(num_components);
		for (int32_t i = 0; i < num_components; ++i)
			a.push_back(components[i].ToJson());
		obj["components"] = std::move(a);
	}
	return obj;
}

bool (*cBot::ms_interaction_functions[IF_NUM])(const sIF_data*) {
	[](const sIF_data* data) -> bool {
		/* Respond to interaction */
		json::object obj;
		switch (data->interaction->GetType()) {
			case INTERACTION_PING:
				return false;
			case INTERACTION_APPLICATION_COMMAND:
				obj["type"] = INTERACTION_CALLBACK_CHANNEL_MESSAGE_WITH_SOURCE;
				break;
			case INTERACTION_MESSAGE_COMPONENT:
				obj["type"] = INTERACTION_CALLBACK_UPDATE_MESSAGE;
				break;
		}
		obj["data"] = data->to_json();
		char path[300];
		sprintf(path, "%s/interactions/%s/%s/callback", DISCORD_API_ENDPOINT, data->interaction->GetId()->ToString(), data->interaction->GetToken());
		return -1 != cNet::PostHttpsRequest(DISCORD_API_HOST, path, data->http_auth, obj);
	},
	[](const sIF_data* data) -> bool {
		/* Edit interaction response */
		if (data->interaction->GetType() == INTERACTION_PING)
			return false;
		char path[512];
		sprintf(path, "%s/webhooks/%s/%s/messages/@original", DISCORD_API_ENDPOINT, data->interaction->GetApplicationId()->ToString(), data->interaction->GetToken());
		return -1 != cNet::PatchHttpsRequest(DISCORD_API_HOST, path, data->http_auth, data->to_json());
	}
};

bool cBot::AcknowledgeInteraction(chInteraction interaction) {
	json::object obj;
	switch (interaction->GetType()) {
		case INTERACTION_PING:
			obj["type"] = INTERACTION_CALLBACK_PONG;
			break;
		case INTERACTION_APPLICATION_COMMAND:
			obj["type"] = INTERACTION_CALLBACK_DEFERRED_CHANNEL_MESSAGE_WITH_SOURCE;
			break;
		case INTERACTION_MESSAGE_COMPONENT:
			obj["type"] = INTERACTION_CALLBACK_DEFERRED_UPDATE_MESSAGE;
			break;
	}
	char path[300];
	sprintf(path, "%s/interactions/%s/%s/callback", DISCORD_API_ENDPOINT, interaction->GetId()->ToString(), interaction->GetToken());
	return -1 != cNet::PostHttpsRequest(DISCORD_API_HOST, path, GetHttpAuthorization(), obj);
}