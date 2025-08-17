#include "GreekBot.h"
#include "DBNicknames.h"
#include "Utils.h"

#include <ranges>
#include <algorithm>

cTask<>
cGreekBot::process_nick_new_member(const cMember& member) HANDLER_BEGIN {
	// Register the new member in the database and delete the old welcoming message if
	if (std::optional msg_id = co_await cNicknamesDAO(co_await cTransaction::New()).Register(member)) try {
		co_await DeleteMessage(LMG_CHANNEL_NICKNAMES_TEST, *msg_id);
	} catch (const xDiscordError& ex) {
		cUtils::PrintErr("Error {} while deleting nickname test message: {}", ex.code().value(), ex.what());
		throw;
	}

	// Send a test message
	co_await CreateMessage(LMG_CHANNEL_NICKNAMES_TEST, cPartialMessage().SetContent(std::format("THIS IS A TEST MESSAGE, PRETEND YOU DIDN'T SEE IT!\n<@{}> just joined.", member.GetUser()->GetId())));
} HANDLER_END

cTask<>
cGreekBot::process_nick_member_update(const cMemberUpdate& member) HANDLER_BEGIN {
	// If the member is not verified, make sure they have no roles and no nicknames, they don't deserve them!
	if (member.IsPending()) {
		auto roles = member.GetRoles();
		auto new_roles = roles
		               | std::views::filter([](auto& id) { return !std::ranges::contains(LMG_PROFICIENCY_ROLES, id); })
		               | std::ranges::to<std::vector>();

		if (!member.GetNickname().empty() || new_roles.size() != roles.size()) {
			cUtils::PrintLog("User {} is not verified, resetting nickname and roles...", member.GetUser().GetId());
			co_await ModifyGuildMember(LMG_GUILD_ID, member.GetUser().GetId(), cMemberOptions().SetNick("").SetRoles(std::move(new_roles)));
		}
		co_return;
	}
} HANDLER_END