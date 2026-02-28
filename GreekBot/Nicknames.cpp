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
	if (std::optional msg_id = co_await cNicknamesDAO(co_await cTransaction::New()).DeleteMessage(member)) try {
		co_await DeleteMessage(LMG_CHANNEL_NEW_MEMBERS, *msg_id);
	} catch (const xDiscordError& ex) {
		if (ex.code() != eDiscordError::UnknownMessage) // If the message is not found (probably already deleted) that's fine!
			throw;
	}
} HANDLER_END

cTask<>
cGreekBot::process_nick_member_update(const cMemberUpdate& member) HANDLER_BEGIN {
	auto txn = co_await cTransaction::New();
	cNicknamesDAO dao(txn);

	try {
		if (member.IsPending()) { // If the member hasn't passed verification...
			auto roles = member.GetRoles();
			auto new_roles = roles
			               | std::views::filter([](auto &id) { return !std::ranges::contains(LMG_PROFICIENCY_ROLES, id); })
			               | std::ranges::to<std::vector>();

			// Remove nickname and proficiency roles if any were assigned (on accident I'd hope...)
			if (!member.GetNickname().empty() || new_roles.size() != roles.size()) {
				cUtils::PrintLog("User {} is not verified, resetting nickname and roles...", member.GetUser().GetId());
				co_await ModifyGuildMember(LMG_GUILD_ID, member.GetUser().GetId(), cMemberOptions().SetNick("").SetRoles(std::move(new_roles)));
			}

			// Also make sure no notification message remains
			co_await txn.Begin();
			if (std::optional msg_id = co_await dao.DeleteMessage(member)) {
				co_await DeleteMessage(LMG_CHANNEL_NEW_MEMBERS, *msg_id);
			}
		} else if (auto member_nick = member.GetNickname(); !member_nick.empty()) { // If the member is verified AND has a nickname...
			// Update the nickname in the database
			// Notice how there is no active transaction is in progress yet; we want the update to happen as eagerly as possible!
			co_await dao.Update(member, member_nick);

			// Update the notification message to notify of nickname change
			co_await txn.Begin();
			if (std::optional msg_id = co_await dao.DeleteMessage(member)) {
				co_await EditMessage(LMG_CHANNEL_NEW_MEMBERS, *msg_id, cMessageUpdate()
					.SetContent(std::format("{:u} Just got a nickname!", member.GetUser().GetId()))
					.SetComponents({
						cActionRow{
							cButton{
								eButtonStyle::Secondary,
								std::format("DLT#{}",
								GetUser().GetId()), // Save the GreekBot id as the author
								"Dismiss"
							}
						}
					})
				);
			}
		} else if (!std::views::filter(member.GetRoles(), [](auto &id) { // If the member is verified and has a proficiency rank but no nickname...
			return std::ranges::contains(LMG_PROFICIENCY_ROLES, id);
		}).empty()) {
			// At this point, we need to restore the original nickname of the member (if applicable) and send a notification message
			co_await txn.Begin();
			if (auto [msg_id, nick] = co_await dao.Get(member); !nick.empty()) { // If there is a nickname registered in the database...
				co_await ModifyGuildMember(LMG_GUILD_ID, member.GetUser().GetId(), cMemberOptions().SetNick(std::move(nick)));

				// Message attributes
				std::string content = std::format("{:u} rejoined and their nickname was restored!", member.GetUser().GetId());
				std::vector components{
					cActionRow{
						cButton{
							eButtonStyle::Secondary,
							std::format("DLT#{}", GetUser().GetId()), // Save the GreekBot id as the author
							"Dismiss"
						}
					}
				};

				// Send a message to notify that the nickname was restored automatically, or edit the existing one
				// Having to edit a leftover message shouldn't ever happen, but we do it just to be safe
				if (msg_id) {
					co_await dao.DeleteMessage(member);
					co_await EditMessage(LMG_CHANNEL_NEW_MEMBERS, *msg_id, cMessageUpdate().SetContent(std::move(content)).SetComponents(std::move(components)));
				} else {
					co_await CreateMessage(LMG_CHANNEL_NEW_MEMBERS, cPartialMessage().SetContent(std::move(content)).SetComponents(std::move(components)));
				}
			} else if (!msg_id) { // If there is no nickname registered in the database and no notification message...
				auto msg = co_await CreateMessage(LMG_CHANNEL_NEW_MEMBERS, cPartialMessage()
					.SetContent(std::format("{:u} just got a rank!", member.GetUser().GetId()))
					.SetComponents({
						cActionRow{
							cButton{
								eButtonStyle::Primary,
								std::format("NCK#{}",
								member.GetUser().GetId()), // Save the member id
								"Assign nickname"
							},
							cButton{
								eButtonStyle::Secondary,
								std::format("DLT#{}",
								GetUser().GetId()), // Save the GreekBot id as the author
								"Dismiss"
							}
						}
					})
				);
				co_await dao.RegisterMessage(member.GetUser(), msg);
			}
		}
	} catch (const xDiscordError& ex) {
		if (ex.code() != eDiscordError::UnknownMessage) // If the message is not found (probably already deleted) that's fine!
			throw;
	}

	// At the end, always commit the transaction; if one wasn't started then this is a harmless no-op
	co_await txn.Commit();
} HANDLER_END

cTask<>
cGreekBot::process_nick_member_remove(const cUser& user) HANDLER_BEGIN {
	// When a user leaves the server, delete any notification messages that may remain
	auto txn = co_await cTransaction::New();
	co_await txn.Begin();
	if (std::optional msg_id = co_await cNicknamesDAO(txn).DeleteMessage(user)) try {
		co_await DeleteMessage(LMG_CHANNEL_NEW_MEMBERS, *msg_id);
	} catch (const xDiscordError& ex) {
		if (ex.code() != eDiscordError::UnknownMessage) // If the message is not found (probably already deleted) that's fine!
			throw;
	}
	co_await txn.Commit();
} HANDLER_END
