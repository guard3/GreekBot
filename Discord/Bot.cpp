#include "Bot.h"
#include "Discord.h"

cTask<>
cBot::OnReady(uhUser user) {
	cUtils::PrintLog("Connected as: %s#%s %s", user->GetUsername(), user->GetDiscriminator(), user->GetId().ToString());
	m_user = std::move(user);
	co_return;
}

cTask<std::vector<cRole>>
cBot::GetGuildRoles(const cSnowflake& guild_id) {
	try {
		json::value v = co_await DiscordGet(cUtils::Format("/guilds/%s/roles", guild_id.ToString()));
		auto& a = v.as_array();
		std::vector<cRole> result;
		result.reserve(a.size());
		for (auto& e : a) {
			result.emplace_back(e);
		}
		co_return result;
	}
	catch (const boost::system::system_error& e) {
		throw xSystemError(e);
	}
}

cTask<cUser>
cBot::GetUser(const cSnowflake &user_id) try {
	co_return cUser(co_await DiscordGet("/users/"s + user_id.ToString()));
}
catch (const boost::system::system_error& e) {
	throw xSystemError(e);
}

cTask<cMember>
cBot::GetGuildMember(const cSnowflake &guild_id, const cSnowflake &user_id) try {
	co_return cMember(co_await DiscordGet(cUtils::Format("/guilds/%s/members/%s", guild_id.ToString(), user_id.ToString())));
}
catch (const boost::system::system_error& e) {
	throw xSystemError(e);
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
		co_await DiscordPatch(cUtils::Format("/guilds/%s/members/%s", guild_id.ToString(), user_id.ToString()), obj);
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