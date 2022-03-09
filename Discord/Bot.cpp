#include "Bot.h"
#include "Discord.h"

void cBot::OnReady(uchUser user) {
	cUtils::PrintLog("Connected as: %s#%s %s", user->GetUsername(), user->GetDiscriminator(), user->GetId().ToString());
	delete m_user;
	m_user = user.release();
}

std::vector<cRole>
cBot::GetGuildRoles(const cSnowflake& guild_id) {
	try {
		json::value v = cDiscord::HttpGet(cUtils::Format("/guilds/%s/roles", guild_id.ToString()), GetHttpAuthorization());
		auto& a = v.as_array();
		std::vector<cRole> result;
		result.reserve(a.size());
		for (auto& e : a) {
			result.emplace_back(e);
		}
		return result;
	}
	catch (const boost::system::system_error& e) {
		throw xSystemError(e);
	}
}

cUser
cBot::GetUser(const cSnowflake &user_id) {
	try {
		return cUser(cDiscord::HttpGet("/users/"s + user_id.ToString(), GetHttpAuthorization()));
	}
	catch (const boost::system::system_error& e) {
		throw xSystemError(e);
	}
}

cMember
cBot::GetGuildMember(const cSnowflake &guild_id, const cSnowflake &user_id) {
	try {
		return cMember(cDiscord::HttpGet(cUtils::Format("/guilds/%s/members/%s", guild_id.ToString(), user_id.ToString()), GetHttpAuthorization()));
	}
	catch (const boost::system::system_error& e) {
		throw xSystemError(e);
	}
}

void
cBot::AddGuildMemberRole(const cSnowflake& guild_id, const cSnowflake& user_id, const cSnowflake &role_id) {
	cDiscord::HttpPut(cUtils::Format("/guilds/%s/members/%s/roles/%s", guild_id.ToString(), user_id.ToString(), role_id.ToString()), GetHttpAuthorization());
}

void
cBot::RemoveGuildMemberRole(const cSnowflake& guild_id, const cSnowflake& user_id, const cSnowflake &role_id) {
	cDiscord::HttpDelete(cUtils::Format("/guilds/%s/members/%s/roles/%s", guild_id.ToString(), user_id.ToString(), role_id.ToString()), GetHttpAuthorization());
}

void
cBot::UpdateGuildMemberRoles(const cSnowflake& guild_id, const cSnowflake& user_id, const std::vector<chSnowflake>& role_ids) {
	try {
		/* Prepare json response */
		json::object obj;
		{
			json::array a;
			a.reserve(role_ids.size());
			for (chSnowflake s: role_ids)
				a.push_back(s->ToString());
			obj["roles"] = std::move(a);
		}
		/* Resolve api path */
		cDiscord::HttpPatch(cUtils::Format("/guilds/%s/members/%s", guild_id.ToString(), user_id.ToString()), GetHttpAuthorization(), obj);
	}
	catch (const boost::system::system_error& e) {
		throw xSystemError(e);
	}
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

void (*cBot::ms_interaction_functions[IF_NUM])(const sIF_data*) {
	[](const sIF_data* data) {
		/* Respond to interaction */
		json::object obj;
		switch (data->interaction->GetType()) {
			case INTERACTION_PING:
				return;
			case INTERACTION_APPLICATION_COMMAND:
				obj["type"] = INTERACTION_CALLBACK_CHANNEL_MESSAGE_WITH_SOURCE;
				break;
			case INTERACTION_MESSAGE_COMPONENT:
				obj["type"] = INTERACTION_CALLBACK_UPDATE_MESSAGE;
				break;
		}
		obj["data"] = data->to_json();
		cDiscord::HttpPost(cUtils::Format("/interactions/%s/%s/callback", data->interaction->GetId().ToString(), data->interaction->GetToken()), data->http_auth, obj);
	},
	[](const sIF_data* data) {
		/* Edit interaction response */
		if (data->interaction->GetType() != INTERACTION_PING) {
			chInteraction i = data->interaction;
			cDiscord::HttpPatch(cUtils::Format("/webhooks/%s/%s/messages/@original", i->GetApplicationId().ToString(), i->GetToken()), data->http_auth, data->to_json());
		}
	},
	[](const sIF_data* data) {
		/* Send followup message */
		if (data->interaction->GetType() != INTERACTION_PING) {
			chInteraction i = data->interaction;
			cDiscord::HttpPost(cUtils::Format("/webhooks/%s/%s", i->GetApplicationId().ToString(), i->GetToken()), data->http_auth, data->to_json());
		}
	}
};

void
cBot::AcknowledgeInteraction(chInteraction interaction) {
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
	cDiscord::HttpPost(cUtils::Format("/interactions/%s/%s/callback", interaction->GetId().ToString(), interaction->GetToken()), GetHttpAuthorization(), obj);
}