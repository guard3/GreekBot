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
	cPartialMessage response;
	/* First, make sure that the invoking user has the appropriate permissions, just for good measure */
	if (!(i.GetMember()->GetPermissions() & PERM_MODERATE_MEMBERS)) {
		co_return co_await InteractionSendMessage(i, response
			.SetFlags(MESSAGE_FLAG_EPHEMERAL)
			.SetContent("You can't do that! You're missing the `MODERATE_MEMBERS` permission.")
		);
	}

	/* Collect parameters */
	hUser pUser;
	hPartialMember pMember;
	std::string_view reason;
	for (auto& option : i.GetOptions()) {
		if (option.GetName() == "user")
			std::tie(pUser, pMember) = option.GetValue<APP_CMD_OPT_USER>();
		else if (option.GetName() == "reason")
			reason = option.GetValue<APP_CMD_OPT_STRING>();
	}

	/* Special message for when anyone tries to warn this bot */
	if (pUser->GetId() == GetUser().GetId()) {
		response.SetFlags(MESSAGE_FLAG_EPHEMERAL).SetContent("Πριτς");
	}
	/* Check for target bot */
	else if (pUser->IsBotUser() || pUser->IsSystemUser()) {
		response.SetFlags(MESSAGE_FLAG_EPHEMERAL).SetContent("You can't warn bots! Bots ενωμένα ποτέ νικημένα!");
	}
	/* Check for target moderator */
	else if (pMember->GetPermissions() & PERM_MODERATE_MEMBERS) {
		response.SetFlags(MESSAGE_FLAG_EPHEMERAL).SetContent("You can't warn a fellow moderator!");
	}
	/* All clear, register the infraction in the database; */
	else [[likely]] {
		co_await InteractionDefer(i, true);
		auto db = co_await BorrowDatabase();
		auto num_infractions = cInfractionsDAO(db).Register(*pUser, i.GetId().GetTimestamp(), reason);
		co_await ReturnDatabase(std::move(db));

		// TODO: Send a proper embed and timeout user
		response.SetContent(std::format("Number of infractions in the last 2 weeks: {}", num_infractions));
	}
	/* Send final confirmation message */
	co_await InteractionSendMessage(i, response);
} HANDLER_END

cTask<>
cGreekBot::process_infractions(cAppCmdInteraction& i) HANDLER_BEGIN {
	/* The response */
	cPartialMessage response;
	/* Collect parameters */
	hUser pUser;
	hPartialMember pMember;
	for (auto& option : i.GetOptions()) {
		if (option.GetName() == "user")
			std::tie(pUser, pMember) = option.GetValue<APP_CMD_OPT_USER>();
	}
	/* If no user parameter is specified, we allow the invoking user to view their infractions... */
	if (!pUser) {
		pUser = &i.GetUser();
		pMember = i.GetMember();
	}
	/* ...otherwise, the invoking user must have the appropriate permissions */
	else if (!(i.GetMember()->GetPermissions() & PERM_MODERATE_MEMBERS)) {
		co_return co_await InteractionSendMessage(i, response
			.SetFlags(MESSAGE_FLAG_EPHEMERAL)
			.SetContent("You can't do that! You are only allowed to view your infractions.")
		);
	}

	infraction_result infs;
	/* If the target member has no immunity, retrieve infractions from the database */
	if (!pUser->IsBotUser() && !pUser->IsSystemUser() && !(pMember->GetPermissions() & PERM_MODERATE_MEMBERS)) {
		co_await InteractionDefer(i, true);
		auto db = co_await BorrowDatabase();
		infs = cInfractionsDAO(db).GetStatsByUser(*pUser, i.GetId().GetTimestamp());
		co_await ReturnDatabase(std::move(db));
	}

	auto& embed = response.EmplaceEmbeds().emplace_back();
	embed.EmplaceAuthor(pUser->GetUsername()).SetIconUrl(cCDN::GetUserAvatar(*pUser));
	embed.SetColor(LMG_COLOR_YELLOW);
	embed.SetTimestamp(i.GetId().GetTimestamp());

	/* Fill the embed with infraction info */
	if (infs.entries.empty()) {
		embed.SetDescription("✅ No infractions found");
	} else {
		auto stat_to_str = [](std::int64_t num) { return std::format("{} infraction{}", num, num == 1 ? "" : "s"); };
		embed.SetDescription("⚠️");
		embed.SetFields({
	        { "Today",     stat_to_str(infs.stats_today),     true },
	        { "This week", stat_to_str(infs.stats_this_week), true },
	        { "Total",     stat_to_str(infs.stats_total),     true },
	        { "Latest infractions", [&] {
				std::string str;
				str.reserve(1024);
				std::size_t n = 0;
				for (auto& [timestamp, reason] : infs.entries)
					std::format_to(back_inserter(str), "{}. {} • <t:{}:R>\n", ++n, reason.empty() ? "`Unspecified`" : reason, floor<std::chrono::seconds>(timestamp).time_since_epoch().count());
				str.pop_back();
				return str;
			} ()}
		});
		/* Add buttons for removing infractions */
		response.SetComponents({
			cActionRow{
				cButton{
					BUTTON_STYLE_SECONDARY,
					std::format("unwarn:{}:{}", pUser->GetId(), i.GetId().GetTimestamp().time_since_epoch().count()),
					"Remove an infraction"
				},
				cButton{
					BUTTON_STYLE_DANGER,
					std::format("unwarn:{}", pUser->GetId()),
					"Remove all infractions"
				}
			}
		});
	}

	co_await InteractionSendMessage(i, response);
} HANDLER_END

cTask<>
cGreekBot::process_infractions_remove(cMsgCompInteraction& i, std::string_view fmt) HANDLER_BEGIN {
	/* First of all, check the invoking member's permissions */
	if (!(i.GetMember()->GetPermissions() & PERM_MODERATE_MEMBERS))
		co_return co_await InteractionSendMessage(i, NO_PERM_MSG);

	cMessageUpdate response;
	/* If the 'Cancel' button was clicked... */
	if (fmt.starts_with("cancel:")) {
		auto& comps = response.EmplaceComponents(i.GetMessage().MoveComponents());
		get<cButton>(comps.at(0).GetComponents().front()).SetLabel("Remove an infraction").SetCustomId(std::format("unwarn:{}", fmt.substr(7)));
		comps.erase(comps.begin() + 1, comps.end());
	}
	/* ...otherwise, if the 'Remove an infraction' button was clicked... */
	else if (auto n = fmt.find(':'); n != std::string_view::npos) {
		using namespace std::chrono;
		/* Collect button id parameters */
		cSnowflake user_id = fmt.substr(0, n);
		auto timestamp = sys_time(milliseconds(cUtils::ParseInt<milliseconds::rep>(fmt.substr(n + 1))));

		/* Collect at most 10 infractions before the timestamp */
		co_await InteractionDefer(i, false);
		auto db = co_await BorrowDatabase();
		auto entries = cInfractionsDAO(db).GetEntriesByUser(user_id, timestamp);
		co_await ReturnDatabase(std::move(db));

		/* Update response with a select menu for each infraction found */
		if (entries.empty()) {
			/* If no infractions are found, update embed and remove all buttons */
			response.EmplaceEmbeds(i.GetMessage().MoveEmbeds()).at(0).SetDescription("✅ No infractions to remove").SetTimestamp(i.GetId().GetTimestamp()).ResetFields();
			response.EmplaceComponents();
		} else {
			// TODO: Get stats and update the original message to the most current state
			auto& comps = response.EmplaceComponents(i.GetMessage().MoveComponents());
			/* Change 'Remove an infraction' to 'Cancel' */
			get<cButton>(comps.at(0).GetComponents().front()).SetLabel("Cancel").SetCustomId(std::format("unwarn:cancel:{}", fmt));
			/* Add a select menu */
			comps.emplace_back(cSelectMenu{
				"INFRACTION_MENU",
				entries | std::views::transform([](infraction_entry& e) {
					return cSelectOption{
						e.reason.empty() ? "Unspecified" : std::move(e.reason),
						std::to_string(e.timestamp.time_since_epoch().count())
					};
				}) | std::ranges::to<std::vector>(),
				"Choose which infraction to remove"
			});
		}
	}
	/* ...otherwise, if the 'Remove all infractions' button was clicked... */
	else {
		cSnowflake user_id = fmt;
		// TODO: use a transaction!
		co_await InteractionDefer(i, false);
		auto db = co_await BorrowDatabase();
		cInfractionsDAO(db).DeleteAll(user_id);
		co_await ReturnDatabase(std::move(db));
		/* Update response; Change embed message and remove all buttons */
		response.EmplaceEmbeds(i.GetMessage().MoveEmbeds()).at(0).SetDescription("✅ All infractions removed").SetTimestamp(i.GetId().GetTimestamp()).ResetFields();
		response.EmplaceComponents();
	}

	co_await InteractionEditMessage(i, response);
} HANDLER_END

cTask<>
cGreekBot::process_infraction_menu(cMsgCompInteraction& i) HANDLER_BEGIN {
	/* Check permissions */
	if (!(i.GetMember()->GetPermissions() & PERM_MODERATE_MEMBERS))
		co_return co_await InteractionSendMessage(i, NO_PERM_MSG);

	/* Delete infraction */
	using namespace std::chrono;
	co_await InteractionDefer(i, true);
	auto db = co_await BorrowDatabase();
	cInfractionsDAO(db).Delete(sys_time(milliseconds(cUtils::ParseInt<milliseconds::rep>(i.GetValues().front()))));
	co_await ReturnDatabase(std::move(db));

	co_await InteractionSendMessage(i, cPartialMessage()
		.SetContent("Infraction removed.")
		.SetComponents({
			cActionRow{
				cButton{
					BUTTON_STYLE_SECONDARY,
					std::format("DLT#{}", i.GetUser().GetId()),
					"Dismiss"
				}
			}
		})
	);
} HANDLER_END