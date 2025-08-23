#include "GreekBot.h"
#include "DBNicknames.h"
#include "Utils.h"

#include <ranges>
#include <algorithm>

cTask<>
cGreekBot::process_nick_new_member(const cMember& member) HANDLER_BEGIN {
	auto txn = co_await cTransaction::New();
	cNicknamesDAO dao(txn);

	// Get the welcoming message id (if there is one left on accident) and register the member
	co_await txn.Begin();
	std::optional msg_id = co_await dao.GetMessage(*member.GetUser());
	co_await dao.Register(member);
	co_await txn.Commit();

	// Try to delete the leftover welcoming message if it exists
	if (msg_id) try {
		co_await DeleteMessage(LMG_CHANNEL_NICKNAMES_TEST, *msg_id);
	} catch (const xDiscordError& ex) {
		if (ex.code() != eDiscordError::UnknownMessage) // If the message is not found (probably already deleted) that's fine!
			throw;
	}

	// Send a test message; TODO: remove or move this to a separate logging utility
	co_await CreateMessage(LMG_CHANNEL_NICKNAMES_TEST, cPartialMessage().SetContent(std::format("THIS IS A TEST MESSAGE, PRETEND YOU DIDN'T SEE IT!\n<@{}> just joined.", member.GetUser()->GetId())));
} HANDLER_END

cTask<>
cGreekBot::process_nick_member_update(const cMemberUpdate& member) HANDLER_BEGIN {
	// If the member hasn't passed verification...
	if (member.IsPending()) {
		auto roles = member.GetRoles();
		auto new_roles = roles
		               | std::views::filter([](auto& id) { return !std::ranges::contains(LMG_PROFICIENCY_ROLES, id); })
		               | std::ranges::to<std::vector>();

		// Remove nickname and proficiency roles if any were assigned (on accident I'd hope...)
		if (!member.GetNickname().empty() || new_roles.size() != roles.size()) {
			cUtils::PrintLog("User {} is not verified, resetting nickname and roles...", member.GetUser().GetId());
			co_await ModifyGuildMember(LMG_GUILD_ID, member.GetUser().GetId(), cMemberOptions().SetNick("").SetRoles(std::move(new_roles)));
		}

		// Also make sure no notification message remains
		auto txn = co_await cTransaction::New();
		cNicknamesDAO dao(txn);
		co_await txn.Begin();
		if (std::optional msg_id = co_await dao.GetMessage(member.GetUser())) try {
			co_await dao.DeleteMessage(member.GetUser());
			co_await DeleteMessage(LMG_CHANNEL_NICKNAMES_TEST, *msg_id);
		} catch (const xDiscordError& ex) {
			if (ex.code() != eDiscordError::UnknownMessage) // If the message is not found (probably already deleted) that's fine!
				throw;
		}
		co_await txn.Commit();
		co_return;
	}

	if (auto nick = member.GetNickname(); !nick.empty()) {
		// Update db if necessary
	} else if (!std::views::filter(member.GetRoles(), [](auto& id) {
		return std::ranges::contains(LMG_PROFICIENCY_ROLES, id);
	}).empty()) {
		// If the member has a nickname and a proficiency role, we need to send a welcoming message if we haven't sent one yet
		auto txn = co_await cTransaction::New();
		cNicknamesDAO dao(txn);

		co_await txn.Begin();
		// TODO: automatically reassign nickname at this point; make a dao function that returns an entire entry
		if (std::optional msg_id = co_await dao.GetMessage(member.GetUser()); !msg_id) {
			auto msg = co_await CreateMessage(LMG_CHANNEL_NICKNAMES_TEST, cPartialMessage()
				.SetContent(std::format("<@{}> just got a rank!", member.GetUser().GetId()))
				.SetComponents({
					cActionRow{
						cButton{
							BUTTON_STYLE_PRIMARY,
							std::format("NCK#{}", member.GetUser().GetId()), // Save the member id
							"Assign nickname"
						},
						cButton{
							BUTTON_STYLE_SECONDARY,
							std::format("DLT#{}", GetUser().GetId()), // Save the GreekBot id as the author
							"Dismiss"
						}
					}
				})
			);
			co_await dao.RegisterMessage(member.GetUser(), msg);
			co_await txn.Commit();
		}
	}
} HANDLER_END

cTask<>
cGreekBot::process_nick_member_remove(const cUser& user) HANDLER_BEGIN {
	// When a user leaves the server, delete any welcoming message that may be remaining
	auto txn = co_await cTransaction::New();
	cNicknamesDAO dao(txn);
	co_await txn.Begin();
	if (std::optional msg_id = co_await dao.GetMessage(user)) try {
		co_await dao.DeleteMessage(user);
		co_await DeleteMessage(LMG_CHANNEL_NICKNAMES_TEST, *msg_id);
	} catch (const xDiscordError& ex) {
		if (ex.code() != eDiscordError::UnknownMessage) // If the message is not found (probably already deleted) that's fine!
			throw;
	}
	co_await txn.Commit();
} HANDLER_END