#include "Bot.h"
#include "Utils.h"
#include "json.h"

cTask<>
cBot::OnReady(uhUser user) {
	cUtils::PrintMsg("Connected as: {} {}", user->GetUsername(), user->GetId());
	m_user = std::move(user);
	co_return;
}

cTask<>
cBot::OnUserUpdate(uhUser user) {
	cUtils::PrintMsg("User updated: {} {}", user->GetUsername(), user->GetId());
	m_user = std::move(user);
	co_return;
}

cTask<std::vector<cRole>>
cBot::GetGuildRoles(const cSnowflake& guild_id) {
	co_return json::value_to<std::vector<cRole>>(co_await DiscordGet(fmt::format("/guilds/{}/roles", guild_id)));
}

cTask<cUser>
cBot::GetUser(const cSnowflake &user_id) {
	co_return cUser{ co_await DiscordGet(fmt::format("/users/{}", user_id)) };
}

cTask<cMember>
cBot::GetGuildMember(const cSnowflake &guild_id, const cSnowflake &user_id) {
	co_return cMember{ co_await DiscordGet(fmt::format("/guilds/{}/members/{}", guild_id, user_id)) };
}

cTask<>
cBot::AddGuildMemberRole(const cSnowflake& guild_id, const cSnowflake& user_id, const cSnowflake &role_id) {
	co_await DiscordPut(fmt::format("/guilds/{}/members/{}/roles/{}", guild_id, user_id, role_id));
}

cTask<>
cBot::RemoveGuildMemberRole(const cSnowflake& guild_id, const cSnowflake& user_id, const cSnowflake &role_id) {
	co_await DiscordDelete(fmt::format("/guilds/{}/members/{}/roles/{}", guild_id, user_id, role_id));
}

cTask<>
cBot::UpdateGuildMemberRoles(const cSnowflake& guild_id, const cSnowflake& user_id, const std::vector<chSnowflake>& role_ids) {
	/* Prepare json response */
	json::object obj;
	json::array& a = obj["roles"].emplace_array();
	a.reserve(role_ids.size());
	for (chSnowflake s: role_ids)
		a.emplace_back(s->ToString());
	/* Resolve api path */
	co_await DiscordPatch(fmt::format("/guilds/{}/members/{}", guild_id, user_id), obj);
}

cTask<>
cBot::modify_guild_member(const cSnowflake& guild_id, const cSnowflake& user_id, const cMemberOptions& options) {
	co_await DiscordPatch(fmt::format("/guilds/{}/members/{}", guild_id, user_id), json::value_from(options).get_object());
}

/* Interaction related functions */
enum eInteractionCallbackType {
	INTERACTION_CALLBACK_PONG                        = 1,         // ACK a Ping
	INTERACTION_CALLBACK_CHANNEL_MESSAGE_WITH_SOURCE = 4,         // respond to an interaction with a message
	INTERACTION_CALLBACK_DEFERRED_CHANNEL_MESSAGE_WITH_SOURCE,    // ACK an interaction and edit a response later, the user sees a loading state
	INTERACTION_CALLBACK_DEFERRED_UPDATE_MESSAGE,                 // for components, ACK an interaction and edit the original message later; the user does not see a loading state
	INTERACTION_CALLBACK_UPDATE_MESSAGE,                          // for components, edit the message the component was attached to
	INTERACTION_CALLBACK_APPLICATION_COMMAND_AUTOCOMPLETE_RESULT, // respond to an autocomplete interaction with suggested choices
	INTERACTION_CALLBACK_MODAL                                    // respond to an interaction with a popup modal
	// 8 TODO: implement autocomplete result interaction stuff... someday
};

template<typename... Ts>
struct visitor : Ts... { using Ts::operator()...; };

template<class... Ts>
visitor(Ts...) -> visitor<Ts...>;

cTask<>
cBot::RespondToInteraction(const cInteraction& i, const cMessageParams& params) {
	int callback = i.Visit<int>(visitor{
		[](const cMsgCompInteraction&) { return INTERACTION_CALLBACK_UPDATE_MESSAGE; },
		[](auto&&) { return INTERACTION_CALLBACK_CHANNEL_MESSAGE_WITH_SOURCE; }
	});
	if (callback == 0)
		co_return;
	json::object obj{{ "type", callback }};
	json::value_from(params, obj["data"]);
	co_await DiscordPost(fmt::format("/interactions/{}/{}/callback", i.GetId(), i.GetToken()), obj);
}

cTask<>
cBot::RespondToInteraction(const cAppCmdInteraction& i, bool) {
	co_await DiscordPost(fmt::format("/interactions/{}/{}/callback", i.GetId(), i.GetToken()), {{ "type", INTERACTION_CALLBACK_DEFERRED_CHANNEL_MESSAGE_WITH_SOURCE }});
}
cTask<>
cBot::RespondToInteraction(const cMsgCompInteraction& i, bool bThinking) {
	co_await DiscordPost(fmt::format("/interactions/{}/{}/callback", i.GetId(), i.GetToken()), {{ "type", bThinking ? INTERACTION_CALLBACK_DEFERRED_CHANNEL_MESSAGE_WITH_SOURCE : INTERACTION_CALLBACK_DEFERRED_UPDATE_MESSAGE }});
}
cTask<>
cBot::RespondToInteraction(const cModalSubmitInteraction& i, bool bThinking) {
	co_await DiscordPost(fmt::format("/interactions/{}/{}/callback", i.GetId(), i.GetToken()), {{ "type", bThinking ? INTERACTION_CALLBACK_DEFERRED_CHANNEL_MESSAGE_WITH_SOURCE : INTERACTION_CALLBACK_DEFERRED_UPDATE_MESSAGE }});
}
cTask<>
cBot::RespondToInteraction(const cInteraction& i, bool bThinking) {
	return i.Visit([this, bThinking](auto& i) {
		return RespondToInteraction(i, bThinking);
	});
}

cTask<>
cBot::RespondToInteractionWithModal(const cInteraction& i, const cModal& modal) {
	json::object obj{{ "type", INTERACTION_CALLBACK_MODAL }};
	json::value_from(modal, obj["data"]);
	co_await DiscordPost(fmt::format("/interactions/{}/{}/callback", i.GetId(), i.GetToken()), obj);
}

cTask<>
cBot::EditInteractionResponse(const cInteraction& i, const cMessageParams& params) {
	co_await DiscordPatch(fmt::format("/webhooks/{}/{}/messages/@original", i.GetApplicationId(), i.GetToken()), json::value_from(params).get_object());
}

cTask<>
cBot::DeleteInteractionResponse(const cInteraction& i) {
	co_await DiscordDelete(fmt::format("/webhooks/{}/{}/messages/@original", i.GetApplicationId(), i.GetToken()));
}

cTask<>
cBot::SendInteractionFollowupMessage(const cInteraction& i, const cMessageParams& params) {
	co_await DiscordPost(fmt::format("/webhooks/{}/{}", i.GetApplicationId(), i.GetToken()), json::value_from(params).get_object());
}

cTask<int>
cBot::BeginGuildPrune(const cSnowflake &id, int days, std::string_view reason) {
	tHttpFields fields;
	if (!reason.empty())
		fields.emplace_back("X-Audit-Log-Reason", cUtils::PercentEncode(reason));
	auto response = co_await DiscordPostNoRetry(fmt::format("/guilds/{}/prune", id), {{ "days", days }}, fields);
	co_return response.at("pruned").to_number<int>();
}

cTask<cChannel>
cBot::CreateDM(const cSnowflake& recipient_id) {
	co_return cChannel {
		co_await DiscordPost("/users/@me/channels", {{ "recipient_id", recipient_id.ToString() }})
	};
}

cTask<cMessage>
cBot::create_message(const cSnowflake& channel_id, const cMessageParams& params) {
	co_return cMessage {
		co_await DiscordPost(fmt::format("/channels/{}/messages", channel_id), json::value_from(params).get_object())
	};
}

cTask<cMessage>
cBot::edit_message(const cSnowflake& channel_id, const cSnowflake& msg_id, const cMessageParams& params) {
	co_return cMessage {
		co_await DiscordPatch(fmt::format("/channels/{}/messages/{}", channel_id, msg_id), json::value_from(params).get_object())
	};
}

cTask<>
cBot::DeleteMessage(const cSnowflake& channel_id, const cSnowflake& msg_id, std::string_view reason) {
	tHttpFields fields;
	if (!reason.empty())
		fields.emplace_back("X-Audit-Log-Reason", cUtils::PercentEncode(reason));
	co_await DiscordDelete(fmt::format("/channels/{}/messages/{}", channel_id, msg_id), fields);
}

cTask<cMessage>
cBot::GetChannelMessage(const cSnowflake& channel_id, const cSnowflake& msg_id) {
	co_return cMessage{
		co_await DiscordGet(fmt::format("/channels/{}/messages/{}", channel_id, msg_id))
	};
}

cTask<>
cBot::RemoveGuildMember(const cSnowflake& guild_id, const cSnowflake& user_id, std::string_view reason) {
	tHttpFields fields;
	if (!reason.empty())
		fields.emplace_back("X-Audit-Log-Reason", cUtils::PercentEncode(reason));
	co_await DiscordDelete(fmt::format("/guilds/{}/members/{}", guild_id, user_id), fields);
}

cTask<>
cBot::CreateGuildBan(const cSnowflake& guild_id, const cSnowflake& user_id, std::chrono::seconds delete_message_seconds, std::string_view reason) {
	tHttpFields fields;
	if (!reason.empty())
		fields.emplace_back("X-Audit-Log-Reason", cUtils::PercentEncode(reason));
	co_await DiscordPut(fmt::format("/guilds/{}/bans/{}", guild_id, user_id), {{ "delete_message_seconds", delete_message_seconds.count() }}, fields);
}

cTask<>
cBot::RemoveGuildBan(const cSnowflake& guild_id, const cSnowflake& user_id, std::string_view reason) {
	tHttpFields fields;
	if (!reason.empty())
		fields.emplace_back("X-Audit-Log-Reason", cUtils::PercentEncode(reason));
	co_await DiscordDelete(fmt::format("/guilds/{}/bans/{}", guild_id, user_id), fields);
}