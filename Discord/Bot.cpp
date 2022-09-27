#include "Bot.h"

cTask<>
cBot::OnReady(uhUser user) {
	cUtils::PrintMsg("Connected as: %s#%s %s", user->GetUsername(), user->GetDiscriminator(), user->GetId().ToString());
	m_user = std::move(user);
	co_return;
}

cTask<std::vector<cRole>>
cBot::GetGuildRoles(const cSnowflake& guild_id) {
	json::value v = co_await DiscordGet(cUtils::Format("/guilds/%s/roles", guild_id.ToString()));
	auto& a = v.as_array();
	std::vector<cRole> result;
	result.reserve(a.size());
	for (auto& e : a) {
		result.emplace_back(e);
	}
	co_return result;
}

cTask<cUser>
cBot::GetUser(const cSnowflake &user_id) {
	co_return cUser(co_await DiscordGet("/users/"s + user_id.ToString()));
}

cTask<cMember>
cBot::GetGuildMember(const cSnowflake &guild_id, const cSnowflake &user_id) {
	co_return cMember(co_await DiscordGet(cUtils::Format("/guilds/%s/members/%s", guild_id.ToString(), user_id.ToString())));
}

cTask<>
cBot::AddGuildMemberRole(const cSnowflake& guild_id, const cSnowflake& user_id, const cSnowflake &role_id) {
	co_await DiscordPut(cUtils::Format("/guilds/%s/members/%s/roles/%s", guild_id.ToString(), user_id.ToString(), role_id.ToString()));
}

cTask<>
cBot::RemoveGuildMemberRole(const cSnowflake& guild_id, const cSnowflake& user_id, const cSnowflake &role_id) {
	co_await DiscordDelete(cUtils::Format("/guilds/%s/members/%s/roles/%s", guild_id.ToString(), user_id.ToString(), role_id.ToString()));
}

cTask<>
cBot::UpdateGuildMemberRoles(const cSnowflake& guild_id, const cSnowflake& user_id, const std::vector<chSnowflake>& role_ids) {
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
	co_await DiscordPatch(cUtils::Format("/guilds/%s/members/%s", guild_id.ToString(), user_id.ToString()), obj);
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

cTask<>
cBot::AcknowledgeInteraction(const cInteraction& i) {
	int callback;
	switch (i.GetType()) {
		default:
			co_return;
		case INTERACTION_PING:
			callback = INTERACTION_CALLBACK_PONG;
			break;
		case INTERACTION_APPLICATION_COMMAND:
			callback = INTERACTION_CALLBACK_DEFERRED_CHANNEL_MESSAGE_WITH_SOURCE;
			break;
		case INTERACTION_MESSAGE_COMPONENT:
			callback = INTERACTION_CALLBACK_DEFERRED_UPDATE_MESSAGE;
	}
	co_await DiscordPost(cUtils::Format("/interactions/%s/%s/callback", i.GetId().ToString(), i.GetToken()), { { "type", callback } });
}
cTask<>
cBot::RespondToInteraction(const cInteraction& interaction, eMessageFlag flags, const cMessageOptions& options) {
	int callback;
	switch (interaction.GetType()) {
		default:
			co_return;
		case INTERACTION_APPLICATION_COMMAND:
			callback = INTERACTION_CALLBACK_CHANNEL_MESSAGE_WITH_SOURCE;
			break;
		case INTERACTION_MESSAGE_COMPONENT:
			callback = INTERACTION_CALLBACK_UPDATE_MESSAGE;
	}
	co_await DiscordPost(cUtils::Format("/interactions/%s/%s/callback", interaction.GetId().ToString(), interaction.GetToken()), {
		{ "type", callback              },
		{ "data", options.ToJson(flags) }
	});
}
cTask<>
cBot::EditInteractionResponse(const cInteraction& interaction, eMessageFlag flags, const cMessageOptions& options) {
	if (interaction.GetType() != INTERACTION_PING)
		co_await DiscordPatch(cUtils::Format("/webhooks/%s/%s/messages/@original", interaction.GetApplicationId().ToString(), interaction.GetToken()), options.ToJson(flags));
}

cTask<>
cBot::SendInteractionFollowupMessage(const cInteraction& interaction, eMessageFlag flags, const cMessageOptions& options) {
	if (interaction.GetType() != INTERACTION_PING)
		co_await DiscordPost(cUtils::Format("/webhooks/%s/%s", interaction.GetApplicationId().ToString(), interaction.GetToken()), options.ToJson(flags));
}

cTask<int>
cBot::BeginGuildPrune(const cSnowflake &id, int days, std::string reason) {
	tHttpFields fields;
	if (!reason.empty())
		fields.emplace_back("X-Audit-Log-Reason", std::move(reason));
	auto response = co_await DiscordPostNoRetry(cUtils::Format("/guilds/%s/prune", id.ToString()), {{ "days", days }}, fields);
	co_return response.at("pruned").as_int64();
}

cTask<cChannel>
cBot::CreateDM(const cSnowflake& recipient_id) {
	co_return cChannel {
		co_await DiscordPost("/users/@me/channels", {{ "recipient_id", recipient_id.ToString() }})
	};
}

cTask<cMessage>
cBot::CreateMessage(const cSnowflake& channel_id, eMessageFlag flags, const cMessageOptions& options) {
	co_return cMessage {
		co_await DiscordPost(cUtils::Format("/channels/%s/messages", channel_id.ToString()), options.ToJson(flags))
	};
}

cTask<cMessage>
cBot::CreateDMMessage(const cSnowflake& recipient_id, eMessageFlag flags, const cMessageOptions& options) {
	co_return co_await CreateMessage((co_await CreateDM(recipient_id)).GetId(), flags, options);
}

cTask<>
cBot::CreateGuildBan(const cSnowflake& guild_id, const cSnowflake& user_id, chrono::seconds delete_message_seconds, const std::string& reason) {
	tHttpFields fields;
	if (!reason.empty())
		fields.emplace_back("X-Audit-Log-Reason", reason);
	co_await DiscordPut(cUtils::Format("/guilds/%s/bans/%s", guild_id.ToString(), user_id.ToString()), {{ "delete_message_seconds", delete_message_seconds.count() }}, fields);
}