#include "Bot.h"
#include "Utils.h"
#include <boost/json.hpp>

namespace json = boost::json;

cTask<>
cBot::OnReady(cUser& user) {
	cUtils::PrintMsg("Connected as: {} {}", user.GetUsername(), user.GetId());
	m_user = std::move(user);
	co_return;
}

cTask<>
cBot::OnUserUpdate(cUser& user) {
	cUtils::PrintMsg("User updated: {} {}", user.GetUsername(), user.GetId());
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
cBot::AddGuildMemberRole(const cSnowflake& guild_id, crefUser user, const cSnowflake &role_id) {
	co_await DiscordPut(fmt::format("/guilds/{}/members/{}/roles/{}", guild_id, user.GetId(), role_id));
}

cTask<>
cBot::RemoveGuildMemberRole(const cSnowflake& guild_id, crefUser user, const cSnowflake &role_id) {
	co_await DiscordDelete(fmt::format("/guilds/{}/members/{}/roles/{}", guild_id, user.GetId(), role_id));
}

cTask<>
cBot::ModifyGuildMember(const cSnowflake& guild_id, const cSnowflake& user_id, const cMemberOptions& options) {
	co_await DiscordPatch(fmt::format("/guilds/{}/members/{}", guild_id, user_id), json::value_from(options).get_object());
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
	co_return cChannel{
		co_await DiscordPost("/users/@me/channels", {{ "recipient_id", recipient_id.ToString() }})
	};
}

cTask<cMessage>
cBot::CreateMessage(crefChannel channel, const cMessageBase& msg) {
	co_return cMessage{
		co_await DiscordPost(fmt::format("/channels/{}/messages", channel.GetId()), json::value_from(msg).get_object())
	};
}

cTask<cMessage>
cBot::CreateDMMessage(const cSnowflake& recipient_id, const cMessageBase& msg) {
	co_return co_await CreateMessage(co_await CreateDM(recipient_id), msg);
}

cTask<cMessage>
cBot::EditMessage(const cSnowflake& channel_id, const cSnowflake& target_msg, const cMessageUpdate& msg) {
	co_return cMessage{
		co_await DiscordPatch(fmt::format("/channels/{}/messages/{}", channel_id, target_msg), json::value_from(msg).get_object())
	};
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