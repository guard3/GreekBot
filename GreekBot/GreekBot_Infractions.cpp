#include "CDN.h"
#include "DBInfractions.h"
#include "GreekBot.h"

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
	        { "Latest infractions", [&entries = infs.entries] {
				auto it = entries.begin(), end = entries.end();
				std::string str = std::format("1. {} • <t:{}:R>", it->reason, floor<std::chrono::seconds>(it->timestamp).time_since_epoch().count());
				for (std::size_t n = 2; ++it != end; ++n)
					std::format_to(back_inserter(str), "\n{}. {} • <t:{}:R>", n, it->reason, floor<std::chrono::seconds>(it->timestamp).time_since_epoch().count());
				return str;
			} ()}
		});
	}

	co_await InteractionSendMessage(i, response);
} HANDLER_END