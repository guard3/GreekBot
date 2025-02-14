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
		const auto now = i.GetId().GetTimestamp();

		co_await InteractionDefer(i, true);
		auto db = co_await BorrowDatabase();
		/*auto num_infractions =*/ cInfractionsDAO(db).Register(*pUser, now, reason);
		co_await ReturnDatabase(std::move(db));

		auto& embed = response.EmplaceEmbeds().emplace_back();
		embed.EmplaceAuthor(std::format("{} was warned ⚠️", pUser->GetUsername())).SetIconUrl(cCDN::GetUserAvatar(*pUser));
		embed.SetColor(LMG_COLOR_YELLOW);
		embed.SetTimestamp(now);
		embed.EmplaceFields().emplace_back("Reason", reason.empty() ? "`Unspecified`" : reason);

		// TODO: Change db Register() result to how many days passed between most recent warns and timeout user
		response.SetComponents({
			cActionRow{
				cButton{
					BUTTON_STYLE_SECONDARY,
					"?",
					"View all infractions",
					true // Temporarily disable this button; TODO: implement
				},
				cButton{
					BUTTON_STYLE_DANGER,
					std::format("unwarn:undo:{}:{}", pUser->GetId(), now.time_since_epoch().count()),
					"Undo"
				}
			}
		});
	}
	/* Send final confirmation message */
	co_await InteractionSendMessage(i, response);
} HANDLER_END

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

	const auto now = i.GetId().GetTimestamp();
	infraction_result infs;
	/* If the target member has no immunity, retrieve infractions from the database */
	if (!pUser->IsBotUser() && !pUser->IsSystemUser() && !(pMember->GetPermissions() & PERM_MODERATE_MEMBERS)) {
		co_await InteractionDefer(i, true);
		auto db = co_await BorrowDatabase();
		infs = cInfractionsDAO(db).GetStatsByUser(*pUser, now);
		co_await ReturnDatabase(std::move(db));
	}

	auto& embed = response.EmplaceEmbeds().emplace_back();
	embed.EmplaceAuthor(pUser->GetUsername()).SetIconUrl(cCDN::GetUserAvatar(*pUser));
	embed.SetColor(LMG_COLOR_YELLOW);
	embed.SetTimestamp(now);

	/* Fill the embed with infraction info */
	if (infs.entries.empty()) {
		embed.SetDescription("✅ No infractions found");
	} else {
		/* Update embed to include stats */
		make_stats(embed, infs);
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

	co_await InteractionSendMessage(i, response);
} HANDLER_END

cTask<>
cGreekBot::process_infractions_remove(cMsgCompInteraction& i, std::string_view fmt) HANDLER_BEGIN {
	/* First of all, check the invoking member's permissions */
	if (!(i.GetMember()->GetPermissions() & PERM_MODERATE_MEMBERS))
		co_return co_await InteractionSendMessage(i, NO_PERM_MSG);

	const auto now = i.GetId().GetTimestamp();
	cMessageUpdate response;
	/* If the 'Cancel' button was clicked... */
	if (fmt.starts_with("cancel:")) {
		auto& comps = response.EmplaceComponents(i.GetMessage().MoveComponents());
		get<cButton>(comps.at(1).GetComponents().front()).SetLabel("Remove an infraction").SetCustomId(std::format("unwarn:{}", fmt.substr(7)));
		comps.front() = std::move(comps[1]);
		comps.erase(comps.begin() + 1, comps.end());
	}
	/* ...otherwise, if an infraction was selected... */
	else if (fmt.starts_with("menu:")) {
		using namespace std::chrono;
		cSnowflake user_id = fmt.substr(5);
		/* Delete selected infraction and retrieve new user stats; TODO: make this a single query */
		co_await InteractionDefer(i, false);
		auto db = co_await BorrowDatabase();
		auto infs = [&] {
			cInfractionsDAO dao(db);
			dao.Delete(user_id, sys_time(milliseconds(cUtils::ParseInt<milliseconds::rep>(i.GetValues().front()))));
			return dao.GetStatsByUser(user_id, now);
		} ();
		co_await ReturnDatabase(std::move(db));

		auto& embed = response.EmplaceEmbeds(i.GetMessage().MoveEmbeds()).at(0).SetTimestamp(now);
		if (infs.entries.empty()) {
			embed.SetDescription("✅ All infractions removed").ResetFields();
			response.EmplaceComponents();
		} else {
			make_stats(embed, infs);
			auto& comps = response.EmplaceComponents(i.GetMessage().MoveComponents());
			get<cButton>(comps.at(1).GetComponents().front()).SetLabel("Remove an infraction").SetCustomId(std::format("unwarn:{}", user_id));
			comps.front() = std::move(comps[1]);
			comps.erase(comps.begin() + 1, comps.end());
		}
	}
	/* ...otherwise, if the 'Remove all infractions' button was clicked... */
	else if (fmt.starts_with("all:")) {
		cSnowflake user_id = fmt.substr(4);
		// TODO: use a transaction!
		co_await InteractionDefer(i, false);
		auto db = co_await BorrowDatabase();
		cInfractionsDAO(db).DeleteAll(user_id);
		co_await ReturnDatabase(std::move(db));
		/* Update response; Change embed message and remove all buttons */
		response.EmplaceEmbeds(i.GetMessage().MoveEmbeds()).at(0).SetDescription("✅ All infractions removed").SetTimestamp(now).ResetFields();
		response.EmplaceComponents();
	}
	/* ...otherwise, if the 'Undo' button was clicked... */
	else if (auto n = fmt.starts_with("undo:") ? fmt.find(':', 5) : std::string_view::npos; n != std::string_view::npos) {
		using namespace std::chrono;
		cSnowflake user_id = fmt.substr(5, n - 5);
		auto timestamp = sys_time(milliseconds(cUtils::ParseInt<milliseconds::rep>(fmt.substr(n + 1))));

		co_await InteractionDefer(i, false);
		auto db = co_await BorrowDatabase();
		cInfractionsDAO(db).Delete(user_id, timestamp);
		co_await ReturnDatabase(std::move(db));

		auto& embed = response.EmplaceEmbeds(i.GetMessage().MoveEmbeds()).at(0);
		auto& author = *embed.SetDescription("✅ Infraction removed").SetTimestamp(now).GetAuthor();
		auto  name = author.GetName();
		author.SetName(name.substr(0, name.rfind(" was")));
		embed.ResetFields();
		response.EmplaceComponents();
	}
	/* ...otherwise, if the 'Remove an infraction' button was clicked... */
	else {
		using namespace std::chrono;
		/* Collect button id parameters */
		cSnowflake user_id = fmt;

		/* Collect at most 10 infractions before the timestamp */
		co_await InteractionDefer(i, false);
		auto db = co_await BorrowDatabase();
		auto infs = cInfractionsDAO(db).GetStatsByUser(user_id, now);
		co_await ReturnDatabase(std::move(db));

		/* Update response with a select menu for each infraction found */
		auto& embed = response.EmplaceEmbeds(i.GetMessage().MoveEmbeds()).at(0).SetTimestamp(now);
		if (infs.entries.empty()) {
			/* If no infractions are found, update embed and remove all buttons */
			embed.SetDescription("✅ No infractions to remove").ResetFields();
			response.EmplaceComponents();
		} else {
			/* Update embed stats */
			make_stats(embed, infs);
			/* Change the 'Remove an infraction' button to 'Cancel' */
			auto& comps = response.EmplaceComponents(i.GetMessage().MoveComponents());
			get<cButton>(comps.at(0).GetComponents().front()).SetLabel("Cancel").SetCustomId(std::format("unwarn:cancel:{}", user_id));
			/* Add a select menu */
			comps.emplace(comps.begin(), cSelectMenu{
				std::format("unwarn:menu:{}", user_id),
				infs.entries | std::views::transform([](infraction_entry& e) {
					return cSelectOption{
						e.reason.empty() ? "Unspecified" : std::move(e.reason),
						std::to_string(e.timestamp.time_since_epoch().count())
					};
				}) | std::ranges::to<std::vector>(),
				"Choose which infraction to remove"
			});
		}
	}

	co_await InteractionEditMessage(i, response);
} HANDLER_END