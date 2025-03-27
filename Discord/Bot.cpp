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
	co_return json::value_to<std::vector<cRole>>(co_await DiscordGet(std::format("/guilds/{}/roles", guild_id)));
}

cTask<cUser>
cBot::GetUser(const cSnowflake &user_id) {
	co_return cUser{ co_await DiscordGet(std::format("/users/{}", user_id)) };
}

cTask<cMember>
cBot::GetGuildMember(const cSnowflake &guild_id, const cSnowflake &user_id) {
	co_return cMember{ co_await DiscordGet(std::format("/guilds/{}/members/{}", guild_id, user_id)) };
}

cTask<>
cBot::AddGuildMemberRole(const cSnowflake& guild_id, crefUser user, const cSnowflake &role_id) {
	co_await DiscordPut(std::format("/guilds/{}/members/{}/roles/{}", guild_id, user.GetId(), role_id));
}

cTask<>
cBot::RemoveGuildMemberRole(const cSnowflake& guild_id, crefUser user, const cSnowflake &role_id) {
	co_await DiscordDelete(std::format("/guilds/{}/members/{}/roles/{}", guild_id, user.GetId(), role_id));
}

cTask<>
cBot::ModifyGuildMember(const cSnowflake& guild_id, const cSnowflake& user_id, const cMemberOptions& options) {
	co_await DiscordPatch(std::format("/guilds/{}/members/{}", guild_id, user_id), json::value_from(options).get_object());
}

cTask<int>
cBot::BeginGuildPrune(const cSnowflake &id, int days, std::string_view reason) {
	std::optional<cHttpField> opt;
	auto fields = reason.empty() ? std::span<cHttpField>() : std::span{ &opt.emplace("X-Audit-Log-Reason", cUtils::PercentEncode(reason)), 1 };
	auto response = co_await DiscordPostNoRetry(std::format("/guilds/{}/prune", id), {{ "days", days }}, fields);
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
		co_await DiscordPost(std::format("/channels/{}/messages", channel.GetId()), json::value_from(msg).get_object())
	};
}

cTask<cMessage>
cBot::CreateDMMessage(const cSnowflake& recipient_id, const cMessageBase& msg) {
	co_return co_await CreateMessage(co_await CreateDM(recipient_id), msg);
}

cTask<cMessage>
cBot::EditMessage(const cSnowflake& channel_id, const cSnowflake& target_msg, const cMessageUpdate& msg) {
	co_return cMessage{
		co_await DiscordPatch(std::format("/channels/{}/messages/{}", channel_id, target_msg), json::value_from(msg).get_object())
	};
}

cTask<cMessage>
cBot::GetChannelMessage(crefChannel channel, crefMessage message) {
	co_return cMessage{
		co_await DiscordGet(std::format("/channels/{}/messages/{}", channel.GetId(), message.GetId()))
	};
}

cTask<>
cBot::RemoveGuildMember(const cSnowflake& guild_id, const cSnowflake& user_id, std::string_view reason) {
	std::optional<cHttpField> opt;
	auto fields = reason.empty() ? std::span<cHttpField>() : std::span{ &opt.emplace("X-Audit-Log-Reason", cUtils::PercentEncode(reason)), 1 };
	co_await DiscordDelete(std::format("/guilds/{}/members/{}", guild_id, user_id), fields);
}

cTask<>
cBot::CreateGuildBan(const cSnowflake& guild_id, const cSnowflake& user_id, std::chrono::seconds delete_message_seconds, std::string_view reason) {
	std::optional<cHttpField> opt;
	auto fields = reason.empty() ? std::span<cHttpField>() : std::span{ &opt.emplace("X-Audit-Log-Reason", cUtils::PercentEncode(reason)), 1 };
	co_await DiscordPut(std::format("/guilds/{}/bans/{}", guild_id, user_id), {{ "delete_message_seconds", delete_message_seconds.count() }}, fields);
}

cTask<>
cBot::RemoveGuildBan(const cSnowflake& guild_id, const cSnowflake& user_id, std::string_view reason) {
	std::optional<cHttpField> opt;
	auto fields = reason.empty() ? std::span<cHttpField>() : std::span{ &opt.emplace("X-Audit-Log-Reason", cUtils::PercentEncode(reason)), 1 };
	co_await DiscordDelete(std::format("/guilds/{}/bans/{}", guild_id, user_id), fields);
}