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
cBot::GetGuildRoles(crefGuild guild) {
	co_return json::value_to<std::vector<cRole>>(co_await DiscordGet(std::format("/guilds/{}/roles", guild.GetId())));
}

cTask<cUser>
cBot::GetUser(crefUser user) {
	co_return cUser{ co_await DiscordGet(std::format("/users/{}", user.GetId())) };
}

cTask<cMember>
cBot::GetGuildMember(crefGuild guild, crefUser user) {
	co_return cMember{ co_await DiscordGet(std::format("/guilds/{}/members/{}", guild.GetId(), user.GetId())) };
}

cTask<>
cBot::AddGuildMemberRole(crefGuild guild, crefUser user, crefRole role) {
	co_await DiscordPut(std::format("/guilds/{}/members/{}/roles/{}", guild.GetId(), user.GetId(), role.GetId()));
}

cTask<>
cBot::RemoveGuildMemberRole(crefGuild guild, crefUser user, crefRole role) {
	co_await DiscordDelete(std::format("/guilds/{}/members/{}/roles/{}", guild.GetId(), user.GetId(), role.GetId()));
}

cTask<>
cBot::ModifyGuildMember(crefGuild guild, crefUser user, const cMemberOptions& options) {
	co_await DiscordPatch(std::format("/guilds/{}/members/{}", guild.GetId(), user.GetId()), json::value_from(options).get_object());
}

cTask<int>
cBot::BeginGuildPrune(crefGuild guild, int days, std::string_view reason) {
	std::optional<cHttpField> opt;
	auto fields = reason.empty() ? std::span<cHttpField>() : std::span{ &opt.emplace("X-Audit-Log-Reason", cUtils::PercentEncode(reason)), 1 };
	auto response = co_await DiscordPostNoRetry(std::format("/guilds/{}/prune", guild.GetId()), {{ "days", days }}, fields);
	co_return response.at("pruned").to_number<int>();
}

cTask<cChannel>
cBot::CreateDM(crefUser recipient) {
	co_return cChannel{
		co_await DiscordPost("/users/@me/channels", {{ "recipient_id", recipient.GetId().ToString() }})
	};
}

cTask<cMessage>
cBot::CreateMessage(crefChannel channel, cMessageView msg) {
	co_return cMessage{
		co_await DiscordPost(std::format("/channels/{}/messages", channel.GetId()), json::value_from(msg).get_object())
	};
}

cTask<cMessage>
cBot::CreateDMMessage(crefUser recipient, cMessageView msg) {
	co_return co_await CreateMessage(co_await CreateDM(recipient), msg);
}

cTask<cMessage>
cBot::EditMessage(crefChannel channel, crefMessage target_msg, const cMessageUpdate& msg) {
	co_return cMessage{
		co_await DiscordPatch(std::format("/channels/{}/messages/{}", channel.GetId(), target_msg.GetId()), json::value_from(msg).get_object())
	};
}

cTask<cMessage>
cBot::GetChannelMessage(crefChannel channel, crefMessage message) {
	co_return cMessage{
		co_await DiscordGet(std::format("/channels/{}/messages/{}", channel.GetId(), message.GetId()))
	};
}

cTask<>
cBot::RemoveGuildMember(crefGuild guild, crefUser user, std::string_view reason) {
	std::optional<cHttpField> opt;
	auto fields = reason.empty() ? std::span<cHttpField>() : std::span{ &opt.emplace("X-Audit-Log-Reason", cUtils::PercentEncode(reason)), 1 };
	co_await DiscordDelete(std::format("/guilds/{}/members/{}", guild.GetId(), user.GetId()), fields);
}

cTask<>
cBot::CreateGuildBan(crefGuild guild, crefUser user, std::chrono::seconds delete_message_seconds, std::string_view reason) {
	std::optional<cHttpField> opt;
	auto fields = reason.empty() ? std::span<cHttpField>() : std::span{ &opt.emplace("X-Audit-Log-Reason", cUtils::PercentEncode(reason)), 1 };
	co_await DiscordPut(std::format("/guilds/{}/bans/{}", guild.GetId(), user.GetId()), {{ "delete_message_seconds", delete_message_seconds.count() }}, fields);
}

cTask<>
cBot::RemoveGuildBan(crefGuild guild, crefUser user, std::string_view reason) {
	std::optional<cHttpField> opt;
	auto fields = reason.empty() ? std::span<cHttpField>() : std::span{ &opt.emplace("X-Audit-Log-Reason", cUtils::PercentEncode(reason)), 1 };
	co_await DiscordDelete(std::format("/guilds/{}/bans/{}", guild.GetId(), user.GetId()), fields);
}