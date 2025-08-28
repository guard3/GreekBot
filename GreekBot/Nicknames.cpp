#include "GreekBot.h"
#include "DBNicknames.h"
#include "Utils.h"

#include <ranges>
#include <algorithm>

cTask<>
cGreekBot::process_nick_new_member(const cMember& member) HANDLER_BEGIN {
	// When a member joins, delete any leftover notification message from the last time they joined (if applicable).
	// Notice how we don't care about any explicit transactions!
	// Once a member joins, the associated message MUST be NULL in the database no matter what
	if (std::optional msg_id = co_await cNicknamesDAO(co_await cTransaction::New()).DeleteMessage(*member.GetUser())) try {
		co_await DeleteMessage(LMG_CHANNEL_NICKNAMES_TEST, *msg_id);
	} catch (const xDiscordError& ex) {
		if (ex.code() != eDiscordError::UnknownMessage) // If the message is not found (probably already deleted) that's fine!
			throw;
	}
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
		// TODO: should this be a separate impl function?
		co_await process_nick_member_remove(member.GetUser());

		co_return;
	}

	if (auto member_nick = member.GetNickname(); !member_nick.empty()) {
		// Update the notification message to notify of nickname change
		auto txn = co_await cTransaction::New();
		cNicknamesDAO dao(txn);

		co_await dao.Update(member.GetUser(), member_nick);

		co_await txn.Begin();
		if (std::optional msg_id = co_await dao.DeleteMessage(member.GetUser())) try {
			co_await EditMessage(LMG_CHANNEL_NICKNAMES_TEST, *msg_id, cMessageUpdate()
				.SetContent(std::format("<@{}> Just got a nickname!", member.GetUser().GetId()))
				.SetComponents({
					cActionRow{
						cButton{
							BUTTON_STYLE_SECONDARY,
							std::format("DLT#{}", GetUser().GetId()), // Save the GreekBot id as the author
							"Dismiss"
						}
					}
				})
			);
		} catch (const xDiscordError& ex) {
			if (ex.code() != eDiscordError::UnknownMessage) // If the message is not found (probably already deleted) that's fine!
				throw;
		}
		co_await txn.Commit();
	} else if (!std::views::filter(member.GetRoles(), [](auto& id) { return std::ranges::contains(LMG_PROFICIENCY_ROLES, id); }).empty()) {
		// If the member has a nickname and a proficiency role, we need to send a welcoming message if we haven't sent one yet
		auto txn = co_await cTransaction::New();
		cNicknamesDAO dao(txn);

		co_await txn.Begin();
		if (auto [msg_id, nick] = co_await dao.GetEntry(member.GetUser()); !nick.empty()) try {
			// If the member has no nickname, but there is one registered in the database, update the member automatically
			co_await ModifyGuildMember(LMG_GUILD_ID, member.GetUser().GetId(), cMemberOptions().SetNick(std::move(nick)));

			// Message attributes
			std::string content = std::format("<@{}> rejoined and their nickname was restored!", member.GetUser().GetId());
			std::vector components{
				cActionRow{
					cButton{
						BUTTON_STYLE_SECONDARY,
						std::format("DLT#{}", GetUser().GetId()), // Save the GreekBot id as the author
						"Dismiss"
					}
				}
			};

			// Send a message to notify that the nickname was restored automatically, or edit the existing one
			// Having to edit a leftover message shouldn't ever happen, but we do it just to be safe
			if (msg_id) {
				co_await dao.DeleteMessage(member.GetUser());
				co_await EditMessage(LMG_CHANNEL_NICKNAMES_TEST, *msg_id, cMessageUpdate().SetContent(std::move(content)).SetComponents(std::move(components)));
			} else {
				co_await CreateMessage(LMG_CHANNEL_NICKNAMES_TEST, cPartialMessage().SetContent(std::move(content)).SetComponents(std::move(components)));
			}
		} catch (const xDiscordError& ex) {
			if (ex.code() != eDiscordError::UnknownMessage) // If the message is not found (probably already deleted) that's fine!
				throw;
		} else if (!msg_id) {
			// If the member has no nickname and no nickname is registered in the database, send a notification message
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
		}
		co_await txn.Commit();
	}
} HANDLER_END

cTask<>
cGreekBot::process_nick_member_remove(const cUser& user) HANDLER_BEGIN {
	// When a user leaves the server, delete any notification messages that may remain
	auto txn = co_await cTransaction::New();
	co_await txn.Begin();
	if (std::optional msg_id = co_await cNicknamesDAO(txn).DeleteMessage(user)) try {
		co_await DeleteMessage(LMG_CHANNEL_NICKNAMES_TEST, *msg_id);
	} catch (const xDiscordError& ex) {
		if (ex.code() != eDiscordError::UnknownMessage) // If the message is not found (probably already deleted) that's fine!
			throw;
	}
	co_await txn.Commit();
} HANDLER_END