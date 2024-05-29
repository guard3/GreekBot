#include "GreekBot.h"
#include <fmt/chrono.h>

cTask<>
cGreekBot::process_prune(cAppCmdInteraction& i) HANDLER_BEGIN {
	cMessageParams response;
	/* Make sure that the user has the necessary permissions */
	if (!(i.GetMember()->GetPermissions() & PERM_KICK_MEMBERS))
		co_return co_await InteractionSendMessage(i, response
			.SetFlags(MESSAGE_FLAG_EPHEMERAL)
			.SetContent("You can't do that. You're missing the `KICK_MEMBERS` permission.")
		);
	/* How many days of inactivity to consider */
	int days = i.GetOptions().empty() ? 2 : i.GetOptions().front().GetValue<APP_CMD_OPT_INTEGER>();
	/* Prune */
	co_await InteractionDefer(i);
	try {
		int pruned = co_await BeginGuildPrune(*i.GetGuildId(), days, "Failed to get a rank");
		response.SetContent(fmt::format("Pruned **{}** member{} for **{}** day{} of inactivity.", pruned, pruned == 1 ? "" : "s", days, days == 1 ? "" : "s"));
	}
	catch (const xRateLimitError& e) {
		response.SetContent(fmt::format("Rate limited. Try again after **{}**.", e.retry_after()));
	}
	/* Send confirmation message */
	co_await InteractionSendMessage(i, response.SetComponents({
		cActionRow{
			cButton{
				BUTTON_STYLE_SECONDARY,
				fmt::format("DLT#{}", i.GetUser().GetId()),
				"Dismiss"
			}
		}
	}));
} HANDLER_END

cTask<>
cGreekBot::process_prune_lmg(cAppCmdInteraction& i) HANDLER_BEGIN {
	using namespace std::chrono;
	using namespace std::chrono_literals;
	/* Check that the invoking member has the appropriate permissions for extra security measure */
	if (!(i.GetMember()->GetPermissions() & PERM_KICK_MEMBERS))
		co_return co_await InteractionSendMessage(i, cMessageParams()
			.SetFlags(MESSAGE_FLAG_EPHEMERAL)
			.SetContent("You can't do that. You're missing the `KICK_MEMBERS` permission.")
		);
	/* Acknowledge */
	co_await InteractionDefer(i);
	/* How many days of inactivity to consider */
	int num_days;
	if (i.GetOptions().empty()) {
		num_days = 2;
	} else {
		num_days = i.GetOptions().front().GetValue<APP_CMD_OPT_INTEGER>();
		if (num_days < 1)
			num_days = 1;
	}
	/* Collect guild information */
	const cGuild& guild = *m_guilds.at(*i.GetGuildId());
	cSnowflake guild_id = guild.GetId();
	std::string guild_name = guild.GetName();
	int total = 0, fails = 0;
	auto started_at = system_clock::now();
	/* Iterate through all members of the guild */
	auto gen = GetGuildMembers(guild_id);
	for (auto it = co_await gen.begin(); it != gen.end(); co_await ++it) {
		cMember& member = *it;
		/* Interaction tokens are valid for 15 minutes, so, just to be safe, abort after 14 minutes have passed */
		if (system_clock::now() - started_at > 14min)
			break;
		/* Select those that have joined for more than x days and have no roles */
		if (auto member_for = started_at - member.JoinedAt(); member_for > num_days * 24h && member.GetRoles().empty()) {
			chUser user = member.GetUser();
			/* Attempt to send a DM explaining the reason of the kick */
			try {
				auto member_for_days = floor<days>(member_for).count();
				co_await CreateDMMessage(user->GetId(), cMessageParams().SetContent(fmt::format(
					"You have been kicked from **{}** because **{}** day{} have passed since you joined and you didn't get a proficiency rank.\n"
					"\n"
					"But don't fret! You are free to rejoin, just make sure to:\n"
					"- Verify your phone number\n"
					"- Get a proficiency rank as mentioned in `#welcoming`\n"
					"https://discord.gg/greek",
					guild_name, member_for_days, member_for_days == 1 ? "" : "s"
				)));
			} catch (...) {}
			/* Then kick */
			try {
				co_await RemoveGuildMember(guild_id, user->GetId(), "Failed to get a rank");
				total++;
			} catch (...) {
				/* If we fail to kick 3 or more members, abort */
				if (++fails > 2)
					break;
			}
		}
	}
	co_await InteractionSendMessage(i, cMessageParams()
		.SetContent(fmt::format("Pruned **{}** member{} for **{}** day{} of inactivity.", total, total == 1 ? "" : "s", num_days, num_days == 1 ? "" : "s"))
		.SetComponents({
			cActionRow{
				cButton{
					BUTTON_STYLE_SECONDARY,
					fmt::format("DLT#{}", i.GetUser().GetId()),
					"Dismiss"
				}
			}
		})
	);
} HANDLER_END