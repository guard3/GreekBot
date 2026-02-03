#include "CDN.h"
#include "DBInfractions.h"
#include "GreekBot.h"
#include "Utils.h"
#include <ranges>

static const auto NO_PERM_MSG = [] {
	cPartialMessage response;
	response.SetFlags(MESSAGE_FLAG_EPHEMERAL).SetContent("You can't do that! You're missing the `MODERATE_MEMBERS` permission.");
	return response;
} ();

cTask<>
cGreekBot::process_warn(cAppCmdInteraction& i) HANDLER_BEGIN {
	/* First, make sure that the invoking user has the appropriate permissions, just for good measure */
	if (i.GetMember()->GetPermissions().TestNone(ePermission::ModerateMembers))
		co_return co_await InteractionSendMessage(i, NO_PERM_MSG);

	/* Collect parameters */
	hUser pUser;
	hPartialMember pMember;
	std::string_view reason;
	if (i.GetCommandType() == APP_CMD_USER) {
		/* If the command was invoked through the context menu, there's only one option: the target user */
		std::tie(pUser, pMember) = i.GetOptions().front().GetValue<APP_CMD_OPT_USER>();
	} else {
		/* Otherwise, go through every option */
		for (auto &option: i.GetOptions()) {
			if (auto name = option.GetName(); name == "user")
				std::tie(pUser, pMember) = option.GetValue<APP_CMD_OPT_USER>();
			else if (name == "reason")
				reason = option.GetValue<APP_CMD_OPT_STRING>();
			else
				throw std::runtime_error(std::format("Unexpected option: {:?}", name));
		}
	}

	/* Test for error conditions */
	if (auto error_msg = [&] -> const char* {
		/* Check that the target user is actually a member */
		if (!pMember)
			return "Warning non-members is kinda pointless, innit?";
		/* Don't warn this bot */
		if (pUser->GetId() == GetUser().GetId())
			return "Πριτς";
		/* Don't let the invoking user warn themselves */
		if (pUser->GetId() == i.GetUser().GetId())
			return "You want to warn yourself? Very humble of you 🤣";
		/* Check for target bot */
		if (pUser->IsBotUser() || pUser->IsSystemUser())
			return "You can't warn bots! Bots ενωμένα ποτέ νικημένα! 🤖";
		/* Check for target moderator */
		if (pMember->GetPermissions().TestAny(ePermission::ModerateMembers))
			return "You can't warn a fellow moderator!";
		/* No error condition, yay! */
		return nullptr;
	} ()) co_return co_await InteractionSendMessage(i, cPartialMessage().SetFlags(MESSAGE_FLAG_EPHEMERAL).SetContent(error_msg));

	/* If the command was invoked through a context menu, send a modal to request a warn reason */
	if (i.GetCommandType() == APP_CMD_USER) {
		co_return co_await InteractionSendModal(i, cModal{
			std::format("warn:{}:{}:{}:{}", pUser->GetId(), pUser->GetUsername(), pUser->GetAvatar(), pUser->GetDiscriminator()),
			std::format("Warn @{}", [pUser, pMember] {
				if (auto name = pMember->GetNick(); !name.empty())
					return name;
				else if (name = pUser->GetGlobalName(); !name.empty())
					return name;
				else
					return pUser->GetUsername();
			} ()),
			{
				cLabel{
					"Reason",
					cTextInput{
						TEXT_INPUT_SHORT,
						"dummy_id", // We don't need a custom id since this is the only component of the modal
						false
					}
				}
			}
		});
	}

	/* Proceed with the regular implementation */
	co_await process_warn_impl(i, pUser->GetId(), pUser->GetUsername(), pUser->GetAvatar(), pUser->GetDiscriminator(), reason);
} HANDLER_END

cTask<>
cGreekBot::process_warn(cModalSubmitInteraction& i, std::string_view fmt) HANDLER_BEGIN {
	/* Check permissions */
	if (i.GetMember()->GetPermissions().TestNone(ePermission::ModerateMembers))
		co_return co_await InteractionSendMessage(i, NO_PERM_MSG);

	/* A simple exception for when the fmt (for whatever reason) is invalid */
	auto invalid_fmt = [fmt] [[noreturn]] { throw std::runtime_error(std::format("Invalid warn modal custom id: {:?}", fmt)); };
	/* Collect parameters from the format string */
	std::size_t p{}, q{};
	if (q = fmt.find(':', p); q == std::string_view::npos) invalid_fmt();
	cSnowflake user_id = fmt.substr(p, q++ - p);
	if (p = fmt.find(':', q); p == std::string_view::npos) invalid_fmt();
	auto username = fmt.substr(q, p++ - q);
	if (q = fmt.find(':', p); q == std::string_view::npos) invalid_fmt();
	auto avatar = fmt.substr(p, q++ - p);
	auto discriminator = cUtils::ParseInt<std::uint16_t>(fmt.substr(q));

	/* Test for error conditions */
	if (auto error_msg = [&] -> const char* {
		/* Make sure we're not warning this bot */
		if (user_id == GetUser().GetId())
			return "Πριτς";
		/* Also make sure that the invoking user isn't warning themselves */
		if (user_id == i.GetUser().GetId())
			return "You want to warn yourself? Very humble of you 🤣";
		/* No error condition, yay! */
		return nullptr;
	} ()) co_return co_await InteractionSendMessage(i, cPartialMessage().SetFlags(MESSAGE_FLAG_EPHEMERAL).SetContent(error_msg));

	/* Proceed with the regular implementation */
	auto& label = std::get<cPartialLabel>(i.GetComponents().front());
	std::string_view reason = get<cTextInput>(label.GetComponent()).GetValue();
	co_await process_warn_impl(i, user_id, username, avatar, discriminator, reason);
} HANDLER_END

cTask<>
cGreekBot::process_warn_impl(cInteraction& i, const cSnowflake &user_id, std::string_view username, std::string_view avatar, std::uint16_t discriminator, std::string_view reason) {
	/* Consider 'now' to be when the current interaction was created */
	const auto now = i.GetId().GetTimestamp();

	/* Prepare the interaction message response */
	cPartialMessage response;
	/* Format the embed of the response */
	auto& embed = response.EmplaceEmbeds().emplace_back().SetTimestamp(now).SetColor(LMG_COLOR_YELLOW);
	auto& author = embed.EmplaceAuthor(std::format("{} was warned ⚠️", username)).SetIconUrl(cCDN::GetUserAvatar(user_id, avatar, discriminator));
	auto& field = embed.EmplaceFields().emplace_back("Reason", reason.empty() ? "`Unspecified`" : reason);
	/* Add 'View all infractions' and 'Undo' buttons */
	response.SetComponents({
		cActionRow{
			cButton{
				BUTTON_STYLE_SECONDARY,
				std::format("infractions:{}", user_id),
				"View all infractions"
			},
			cButton{
				BUTTON_STYLE_DANGER,
				std::format("unwarn:undo:{}:{}", user_id, now.time_since_epoch().count()),
				"Undo"
			}
		}
	});

	/* Acknowledge interaction since accessing the database may be slow */
	co_await InteractionDefer(i, true);
	auto txn = co_await cTransaction::New();
	cInfractionsDAO dao(txn);

	/* Register new infraction and calculate the delta between the 2 most recent infractions */
	co_await txn.Begin();
	co_await dao.Register(user_id, now, reason);
	auto dt = co_await dao.GetRecentDeltaTime(user_id);
	/* Send confirmation message */
	co_await InteractionSendMessage(i, response);
	co_await txn.Commit();

	/* Time out the user if the 2 most recent infractions were added in less than 3 months */
	using namespace std::chrono;
	if (dt < months(3)) {
		/* How long to time out for? */
		int timeout_days = [dt] {
			if (dt < days( 3)) return 7;
			if (dt < days( 5)) return 5;
			if (dt < days(15)) return 3;
			return 1;
		} ();

		/* Edit response to reflect timeout reason */
		embed.SetDescription(std::format("User timed out for **{}** day{}", timeout_days, timeout_days == 1 ? "" : "s"));
		author.SetName(username);
		if (std::string& rsn = field.EmplaceValue("Was warned multiple times"); dt <= days(1))
			rsn += " in a day";
		else if (dt <= months(1))
			std::format_to(back_inserter(rsn), " within {} days", ceil<days>(dt).count());
		else
			std::format_to(back_inserter(rsn), " within {} months", ceil<months>(dt).count());

		/* Add a button to remove timeout */
		response.SetComponents({
			cActionRow{
				cButton{
					BUTTON_STYLE_DANGER,
					std::format("timeout:{}", user_id),
					"Remove timeout"
				}
			}
		});

		/* Register timeout in the database */
		co_await txn.Begin();
		co_await dao.TimeOut(user_id, now);
		/* Timeout member */
		co_await ModifyGuildMember(*i.GetGuildId(), user_id, cMemberOptions().SetCommunicationsDisabledUntil(now + days(timeout_days)));
		co_await txn.Commit();
		/* Send confirmation message */
		co_await InteractionSendMessage(i, response);
	}
}

static void make_stats(cEmbed& embed, const infraction_result& res) {
	auto stat_to_str = [](std::int64_t num) { return std::format("{} infraction{}", num, num == 1 ? "" : "s"); };
	embed.SetDescription("⚠️");
	embed.SetFields({
		{ "Today",     stat_to_str(res.stats_today),     true },
		{ "This week", stat_to_str(res.stats_this_week), true },
		{ "Total",     stat_to_str(res.stats_total),     true },
		{ "Latest infractions", [&] {
			std::string str;
			str.reserve(1024);
			std::size_t n = 0;
			for (auto&[timestamp, reason] : res.entries)
				std::format_to(back_inserter(str), "{}. {} • <t:{}:R>\n", ++n, reason.empty() ? "`Unspecified`" : reason, floor<std::chrono::seconds>(timestamp).time_since_epoch().count());
			return str;
		} ()}
	});
}

cTask<>
cGreekBot::process_infractions(cAppCmdInteraction& i) HANDLER_BEGIN {
	/* The response */
	cPartialMessage response;
	/* Collect parameters */
	hUser pUser;
	hPartialMember pMember;
	if (i.GetCommandType() == APP_CMD_USER) {
		std::tie(pUser, pMember) = i.GetOptions().front().GetValue<APP_CMD_OPT_USER>();
	} else {
		for (auto &option: i.GetOptions()) {
			if (option.GetName() == "user")
				std::tie(pUser, pMember) = option.GetValue<APP_CMD_OPT_USER>();
		}
	}
	/* If no user parameter is specified, we allow the invoking user to view their infractions... */
	if (!pUser) {
		pUser = &i.GetUser();
		pMember = i.GetMember();
	}
	/* ...otherwise, the invoking user must have the appropriate permissions */
	else if (i.GetMember()->GetPermissions().TestNone(ePermission::ModerateMembers)) {
		co_return co_await InteractionSendMessage(i, response
			.SetFlags(MESSAGE_FLAG_EPHEMERAL)
			.SetContent("You can't do that! You are only allowed to view your own infractions.")
		);
	}

	const auto now = i.GetId().GetTimestamp();
	/* Retrieve infraction stats from the database */
	co_await InteractionDefer(i, true);
	auto stats = co_await cInfractionsDAO(co_await cTransaction::New()).GetStatsByUser(*pUser, now);

	auto& embed = response.EmplaceEmbeds().emplace_back();
	embed.EmplaceAuthor(pUser->GetUsername()).SetIconUrl(cCDN::GetUserAvatar(*pUser));
	embed.SetColor(LMG_COLOR_YELLOW);
	embed.SetTimestamp(now);

	/* Fill the embed with infraction info */
	if (stats.entries.empty()) {
		embed.SetDescription("✅ No infractions found");
	} else {
		/* Update embed to include stats */
		make_stats(embed, stats);
		/* Add buttons for removing infractions */
		response.SetComponents({
			cActionRow{
				cButton{
					BUTTON_STYLE_SECONDARY,
					std::format("unwarn:{}", pUser->GetId()),
					"Remove an infraction"
				},
				cButton{
					BUTTON_STYLE_DANGER,
					std::format("unwarn:all:{}", pUser->GetId()),
					"Remove all infractions"
				}
			}
		});
	}

	if (!pMember) {
		co_await ResumeOnEventStrand();
		embed.EmplaceFooter(std::format("Not a member of {}{}", m_guilds.at(*i.GetGuildId()).GetName(), stats.entries.empty() ? "" : " anymore"));
	}

	co_await InteractionSendMessage(i, response);
} HANDLER_END

cTask<>
cGreekBot::process_infractions_button(cMsgCompInteraction& i, cSnowflake user_id) HANDLER_BEGIN {
	/* We only allow a moderator or the offending user to view infractions */
	if (i.GetUser().GetId() != user_id && i.GetMember()->GetPermissions().TestNone(ePermission::ModerateMembers)) {
		co_return co_await InteractionSendMessage(i, cPartialMessage()
			.SetFlags(MESSAGE_FLAG_EPHEMERAL)
			.SetContent("You can't do that! You are only allowed to view your own infractions.")
		);
	}
	const auto now = i.GetId().GetTimestamp();
	/* Get user stats from the database */
	co_await InteractionDefer(i);
	auto stats = co_await cInfractionsDAO(co_await cTransaction::New()).GetStatsByUser(user_id, now);

	/* Prepare a response by keeping just the author's username and removing all buttons */
	cMessageUpdate response;
	auto& comps = response.EmplaceComponents();
	auto& embed = response.EmplaceEmbeds(i.GetMessage().MoveEmbeds()).at(0).SetTimestamp(now);
	auto& author = *embed.GetAuthor();
	auto name = author.GetName();
	author.SetName(name.substr(0, name.rfind(" was")));

	if (stats.entries.empty()) {
		/* Simple confirmation message if no infractions found */
		embed.SetDescription("✅ No infractions found").ResetFields();
	} else {
		/* Write stats to the embed */
		make_stats(embed, stats);
		/* Add 'Remove an infraction' and 'Remove all infractions' buttons */
		comps.emplace_back(cButton{
			BUTTON_STYLE_SECONDARY,
			std::format("unwarn:{}", user_id),
			"Remove an infraction"
		}, cButton {
			BUTTON_STYLE_DANGER,
			std::format("unwarn:all:{}", user_id),
			"Remove all infractions"
		});
	}
	/* Update original message */
	co_await InteractionEditMessage(i, response);
} HANDLER_END

cTask<>
cGreekBot::process_infractions_remove(cMsgCompInteraction& i, std::string_view fmt) HANDLER_BEGIN {
	using namespace std::chrono;
	/* First of all, check the invoking member's permissions */
	if (i.GetMember()->GetPermissions().TestNone(ePermission::ModerateMembers))
		co_return co_await InteractionSendMessage(i, NO_PERM_MSG);

	const auto now = i.GetId().GetTimestamp();
	cMessageUpdate response;
	/* If the 'Cancel' button was clicked... */
	if (fmt.starts_with("cancel:")) {
		auto& comps = response.EmplaceComponents(i.GetMessage().MoveComponents());
		get<cButton>(comps.at(1).GetComponents().front()).SetLabel("Remove an infraction").SetCustomId(std::format("unwarn:{}", fmt.substr(7)));
		comps.front() = std::move(comps[1]);
		comps.erase(comps.begin() + 1, comps.end());

		co_await InteractionEditMessage(i, response);
		co_return;
	}

	/* In all other cases, accessing the database is required */
	co_await InteractionDefer(i, false);
	auto txn = co_await cTransaction::New();
	cInfractionsDAO dao(txn);
	co_await txn.Begin();

	/* If an infraction was selected to be removed... */
	if (fmt.starts_with("menu:")) {
		/* Collect parameters */
		cSnowflake user_id = fmt.substr(5);
		/* Delete selected infraction and retrieve updated user stats */
		co_await dao.Delete(user_id, sys_time(milliseconds(cUtils::ParseInt<milliseconds::rep>(i.GetValues().front()))));
		auto stats = co_await dao.GetStatsByUser(user_id, now);
		/* Update embed and buttons */
		auto& embed = response.EmplaceEmbeds(i.GetMessage().MoveEmbeds()).at(0).SetTimestamp(now);
		if (stats.entries.empty()) {
			embed.SetDescription("✅ All infractions removed").ResetFields();
			response.EmplaceComponents();
		} else {
			make_stats(embed, stats);
			auto& comps = response.EmplaceComponents(i.GetMessage().MoveComponents());
			get<cButton>(comps.at(1).GetComponents().front()).SetLabel("Remove an infraction").SetCustomId(std::format("unwarn:{}", user_id));
			comps.front() = std::move(comps[1]);
			comps.erase(comps.begin() + 1, comps.end());
		}
	}
	/* ...otherwise, if the 'Remove all infractions' button was clicked... */
	else if (fmt.starts_with("all:")) {
		/* Collect parameters */
		cSnowflake user_id = fmt.substr(4);
		/* Delete all infractions of the user */
		co_await dao.DeleteAll(user_id);
		/* Update response; Change embed message and remove all buttons */
		response.EmplaceEmbeds(i.GetMessage().MoveEmbeds()).at(0).SetDescription("✅ All infractions removed").SetTimestamp(now).ResetFields();
		response.EmplaceComponents();
	}
	/* ...otherwise, if the 'Undo' button was clicked... */
	else if (auto n = fmt.starts_with("undo:") ? fmt.find(':', 5) : std::string_view::npos; n != std::string_view::npos) {
		/* Collect parameters */
		cSnowflake user_id = fmt.substr(5, n - 5);
		auto timestamp = sys_time(milliseconds(cUtils::ParseInt<milliseconds::rep>(fmt.substr(n + 1))));
		/* Delete the selected infraction */
		co_await dao.Delete(user_id, timestamp);
		/* Update response; Change embed message and remove all buttons */
		auto& embed = response.EmplaceEmbeds(i.GetMessage().MoveEmbeds()).at(0);
		auto& author = *embed.SetDescription("✅ Infraction removed").SetTimestamp(now).GetAuthor();
		auto  name = author.GetName();
		author.SetName(name.substr(0, name.rfind(" was")));
		embed.ResetFields();
		response.EmplaceComponents();
	}
	/* ...otherwise, if the 'Remove an infraction' button was clicked... */
	else {
		/* Collect parameters */
		cSnowflake user_id = fmt;
		/* Retrieve the 10 most recent infractions of the user */
		auto stats = co_await dao.GetStatsByUser(user_id, now);
		/* Update response with a select menu for each infraction found */
		auto& embed = response.EmplaceEmbeds(i.GetMessage().MoveEmbeds()).at(0).SetTimestamp(now);
		if (stats.entries.empty()) {
			/* If no infractions are found, update embed and remove all buttons */
			embed.SetDescription("✅ No infractions to remove").ResetFields();
			response.EmplaceComponents();
		} else {
			/* Update embed stats */
			make_stats(embed, stats);
			/* Change the 'Remove an infraction' button to 'Cancel' */
			auto& comps = response.EmplaceComponents(i.GetMessage().MoveComponents());
			get<cButton>(comps.at(0).GetComponents().front()).SetLabel("Cancel").SetCustomId(std::format("unwarn:cancel:{}", user_id));
			/* Add a select menu */
			comps.emplace(comps.begin(), cSelectMenu{
				std::format("unwarn:menu:{}", user_id),
				stats.entries | std::views::transform([](infraction_entry& e) {
					return cSelectOption{
						e.reason.empty() ? "Unspecified" : std::move(e.reason),
						std::to_string(e.timestamp.time_since_epoch().count())
					};
				}) | std::ranges::to<std::vector>(),
				"Choose which infraction to remove"
			});
		}
	}
	/* Update original message and do database cleanup */
	co_await InteractionEditMessage(i, response);
	co_await txn.Commit();
} HANDLER_END

cTask<>
cGreekBot::process_timeout_remove(cMsgCompInteraction& i, cSnowflake user_id) HANDLER_BEGIN {
	/* Check permissions */
	if (i.GetMember()->GetPermissions().TestNone(ePermission::ModerateMembers))
		co_return co_await InteractionSendMessage(i, NO_PERM_MSG);

	cMessageUpdate response;
	response.EmplaceEmbeds(i.GetMessage().MoveEmbeds()).at(0).SetDescription("✅ Timeout removed").SetTimestamp(i.GetId().GetTimestamp()).ResetFields();
	response.EmplaceComponents();

	co_await ModifyGuildMember(*i.GetGuildId(), user_id, cMemberOptions().SetCommunicationsDisabledUntil({}));
	co_await InteractionEditMessage(i, response);
} HANDLER_END