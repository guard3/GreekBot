#include "GreekBot.h"
#include <fmt/format.h>
#include <fmt/chrono.h>

cTask<>
cGreekBot::OnInteraction_prune(const cInteraction& i) {
	/* Acknowledge interaction first */
	co_await RespondToInteraction(i);
	try {
		/* Make sure that we're on a guild and that we have the necessary permissions */
		chSnowflake guild_id = i.GetGuildId();
		chMember member = i.GetMember();
		if (!(member->GetPermissions() & PERM_KICK_MEMBERS))
			co_return co_await EditInteractionResponse(i, kw::content="You can't do that. You're missing the `KICK_MEMBERS` permission.");
		/* How many days of inactivity to consider */
		auto& options = i.GetData<INTERACTION_APPLICATION_COMMAND>().Options;
		int days = options.empty() ? 2 : options.front().GetValue<APP_CMD_OPT_INTEGER>();
		/* Prune */
		std::string str;
		try {
			int pruned = co_await BeginGuildPrune(*guild_id, days, "Failed to get a rank");
			str = fmt::format("Pruned **{}** member{} for **{}** day{} of inactivity.", pruned, pruned == 1 ? "" : "s", days, days == 1 ? "" : "s");
		}
		catch (const xRateLimitError& e) {
			str = fmt::format("Rate limited. Try again after **{}**.", e.retry_after());
		}
		/* Send confirmation message */
		co_return co_await EditInteractionResponse(
			i,
			kw::content=std::move(str),
			kw::components = {
				cActionRow{
					cButton<BUTTON_STYLE_SECONDARY>{
						fmt::format("DLT#{}", member->GetUser()->GetId()),
						kw::label = "Dismiss"
					}
				}
			}
		);
	}
	catch (...) {}
	co_await EditInteractionResponse(i, kw::content="An unexpected error has occurred, try again later.");
}

cTask<>
cGreekBot::OnInteraction_prune_lmg(const cInteraction& i) {
	/* Check that the invoking member has the appropriate permissions for extra security measure */
	chMember invoking_member = i.GetMember();
	if (!(invoking_member->GetPermissions() & PERM_KICK_MEMBERS))
		co_return co_await RespondToInteraction(i, kw::flags=MESSAGE_FLAG_EPHEMERAL, kw::content="You can't do that. You're missing the `KICK_MEMBERS` permission.");
	/* Otherwise, carry on normally */
	co_await RespondToInteraction(i);
	auto before = chrono::system_clock::now();
	try {
		/* Collect guild information */
		const cGuild& guild = *m_guilds[*i.GetGuildId()];
		cSnowflake guild_id = guild.GetId();
		std::string guild_name = guild.GetName();
		int total = 0, fails = 0;
		/* Get all members of the guild */
		for (auto gen = GetGuildMembers(guild_id); co_await gen.HasValue();) {
			cMember member = co_await gen();
			/* Interaction tokens are valid for 15 minutes, so, just to be safe, abort after 14 minutes have passed*/
			auto now = chrono::system_clock::now();
			if (now - before > 14min)
				break;
			/* Select those that have joined for more than 2 days and have no roles */
			if (auto member_for = now - member.JoinedAt(); member_for > 48h && member.GetRoles().empty()) {
				chUser user = member.GetUser();
				/* Attempt to send a DM explaining the reason of the kick */
				try {
					co_await CreateDMMessage(
						user->GetId(),
						kw::content=fmt::format(
							"You have been kicked from **{}** because **{}** days have passed since you joined and you didn't get a proficiency rank.\n"
							"\n"
							"But don't fret! You are free to rejoin, just make sure to:\n"
							"- Verify your phone number\n"
							"- Get a proficiency rank as mentioned in `#welcoming`\n"
							"https://discord.gg/greek",
							guild_name, chrono::duration_cast<chrono::days>(member_for).count())
					);
				}
				catch (...) {}
				/* Then kick */
				try {
					co_await RemoveGuildMember(guild_id, user->GetId(), "Failed to get a rank");
					total++;
				}
				catch (...) {
					if (++fails > 2)
						break;
				}
			}
		}
		co_return co_await EditInteractionResponse(i,
			kw::content=fmt::format("Pruned **{}** member{}", total, total == 1 ? "" : "s"),
			kw::components={
				cActionRow {
					cButton<BUTTON_STYLE_SECONDARY> {
						fmt::format("DLT#{}", invoking_member->GetUser()->GetId()),
						kw::label="Dismiss"
					}
				}
			}
		);
	}
	catch (const std::exception& e) {
		cUtils::PrintErr("{}", e.what());
	}
	catch (...) {}
	co_await EditInteractionResponse(i, kw::content="An unexpected error has occurred, try again later.");
}