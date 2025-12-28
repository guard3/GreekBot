#include "Database.h"
#include "DBMessageLog.h"
#include "DBTempBans.h"
#include "DBNicknames.h"
#include "GreekBot.h"
#include "Utils.h"

cTask<>
cGreekBot::OnHeartbeat() try {
	using namespace std::chrono;
	using namespace std::chrono_literals;
	/* The following actions will be taking place once every few hours */
	if (auto now = steady_clock::now(); now - m_before > 3h) {
		m_before = now;
		/* Create a transaction */
		auto txn = co_await cTransaction::New();

		// Cleanup old logged messages
		if (auto num = co_await cMessageLogDAO(txn).Cleanup(); num > 0) {
			cUtils::PrintLog("Cleaned up {} old logged message{}", num, num == 1 ? "" : "s");
		}

		// Remove any stale nickname entries
		if (auto num = co_await cNicknamesDAO(txn).Cleanup(); num > 0) {
			cUtils::PrintLog("Cleaned up {} entr{} for members who joined, got no nickname and left", num, num == 1 ? "y" : "ies");
		}

		/* Remove expired bans */
		std::vector<cSnowflake> expired = co_await cTempBanDAO(txn).GetExpired();
		std::vector<cSnowflake> unbanned;
		unbanned.reserve(expired.size());
		cPartialMessage msg;
		for (const cSnowflake& user_id : expired) try {
			co_await RemoveGuildBan(LMG_GUILD_ID, user_id, "Temporary ban expired");
			unbanned.push_back(user_id);
			co_await CreateMessage(LMG_CHANNEL_USER_LOG, msg.SetContent(std::format("<@{}>'s ban expired", user_id)));
		} catch (const xDiscordError&) {
			/* Ban not found but that's fine! */
		}
		/* If at least one user was unbanned, remove them from the database */
		if (!unbanned.empty()) {
			co_await cTempBanDAO(txn).Remove(unbanned);
			cUtils::PrintLog("{} ban{} expired", unbanned.size(), unbanned.size() == 1 ? "" : "s");
		}
		/* Prune */
		std::size_t total_pruned = 0;

		cMemberOptions memberUpdate;
		auto& new_roles = memberUpdate.EmplaceRoles();

		try {
			co_await ResumeOnEventStrand();
			std::string guild_name = m_guilds.at(LMG_GUILD_ID).GetName();
			auto started_at = system_clock::now();
			/* Iterate through all members of the guild */
			co_for (auto& member, RequestGuildMembers(LMG_GUILD_ID)) {
				/* We only care about those who have joined for 2 days or more and still have no roles */
				if (auto member_for = floor<days>(started_at - member.JoinedAt()); member_for >= days(2) && member.GetRoles().empty()) {
					auto& user = *member.GetUser();
					/* Attempt to send a DM explaining the reason of the kick */
					try {
						msg.SetContent(std::format("You have been kicked from **{}** because **{:%Q}** day{} have passed since you joined and you didn't get a proficiency rank.\n\n"
						                           "But don't fret! You are free to rejoin, just make sure to:\n"
						                           "- Verify your phone number\n"
						                           "- Get a proficiency rank as mentioned in `#welcoming`\n"
						                           "https://discord.gg/greek", guild_name, member_for, member_for == days(1) ? "" : "s"));
						co_await CreateDMMessage(user, msg);
					} catch (...) {}
					/* Then kick */
					co_await RemoveGuildMember(LMG_GUILD_ID, user, "Failed to get a rank");
					++total_pruned;
				} else if (member.PremiumSince() == system_clock::time_point{} && member.GetPermissions().TestNone(PERM_MANAGE_ROLES)) { // If the (non-admin) member is not a nitro booster...
					auto& user = *member.GetUser();
					// Copy the member roles to the member update object, filtering out any color roles
					auto roles = member.GetRoles();
					new_roles.clear();
					std::ranges::copy_if(roles, std::back_inserter(new_roles), [](auto& role_id) { return !std::ranges::contains(LMG_NITRO_BOOSTER_COLOR_ROLES, role_id); });

					// Remove the color role if one exists
					if (roles.size() != new_roles.size()) try {
						co_await ModifyGuildMember(LMG_GUILD_ID, member, memberUpdate);
						cUtils::PrintLog("{} with id {} stopped nitro boosting. Their color role was removed!", user.GetUsername(), user.GetId());
					} catch (const xDiscordError& ex) {
						cUtils::PrintErr("{} with id {} stopped nitro boosting. Their color role was not removed: {}", user.GetUsername(), user.GetId(), ex.what());
					}
				}
			}
		} catch (...) {
			/* If any exception is thrown, that's ok; we can retry later */
		}
		/* Report how many members were pruned */
		if (total_pruned) {
			cUtils::PrintLog("Pruned {} member{}", total_pruned, total_pruned == 1 ? "" : "s");
			co_await CreateMessage(LMG_CHANNEL_USER_LOG, msg.SetContent(std::format("Pruned **{}** member{}", total_pruned, total_pruned == 1 ? "" : "s")));
		}
	}
} catch (const std::exception& e) {
	cUtils::PrintErr("An unhandled exception escaped the heartbeat handler: {}", e.what());
} catch (...) {
	cUtils::PrintErr("An unhandled exception escaped the heartbeat handler.");
}