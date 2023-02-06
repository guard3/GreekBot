#include "GreekBot.h"

cTask<>
cGreekBot::OnInteraction_prune(const cInteraction& i) {
	/* Acknowledge interaction first */
	co_await RespondToInteraction(i);
	try {
		/* Make sure that we're on a guild and that we have the necessary permissions */
		chSnowflake guild_id = i.GetGuildId();
		chMember member = i.GetMember();
		if (!guild_id || !member) throw 0;
		if (!(member->GetPermissions() & PERM_KICK_MEMBERS))
			co_return co_await EditInteractionResponse(i, kw::content="You can't do that. You're missing the `KICK_MEMBERS` permission.");
		/* How many days of inactivity to consider */
		auto &options = i.GetData<INTERACTION_APPLICATION_COMMAND>()->Options;
		int days = options.empty() ? 2 : options.front().GetValue<APP_CMD_OPT_INTEGER>();
		/* Prune */
		std::string str;
		try {
			int pruned = co_await BeginGuildPrune(*guild_id, days, "Failed to get a rank");
			str = cUtils::Format("Pruned **%d** member%s for **%d** day%s of inactivity.", pruned, pruned == 1 ? "" : "s", days, days == 1 ? "" : "s");
		}
		catch (const xRateLimitError& e) {
			str = cUtils::Format("Rate limited. Try again after **%dms**.", (int)e.retry_after().count());
		}
		/* Send confirmation message */
		co_return co_await EditInteractionResponse(
			i,
			kw::content=std::move(str),
			kw::components = {
				cActionRow{
					cButton<BUTTON_STYLE_SECONDARY>{
						cUtils::Format("DLT#%s", member->GetUser()->GetId().ToString()),
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
	/* Respond with a message because this may take too long */
	co_await RespondToInteraction(i, kw::content="Please wait, this may take a while...");
	try {
		//TODO: add more error checking later
		/* Collect guild information */
		const cGuild& guild = *m_guilds[*i.GetGuildId()];
		cSnowflake guild_id = guild.GetId();
		std::string guild_name = guild.GetName();
		int total = 0, fails = 0;
		/* Get all members of the guild */
		for (auto gen = GetGuildMembers(guild_id); co_await gen.HasValue();) {
			cMember member = co_await gen();
			/* Select those that have joined for more than 2 days and have no roles */
			if (auto member_for = chrono::system_clock::now() - member.JoinedAt(); member_for > 48h && member.Roles.empty()) {
				chUser user = member.GetUser();
				/* Attempt to send a DM explaining the reason of the kick */
				try {
					co_await CreateDMMessage(
						user->GetId(),
						kw::content="You have been kicked from **" + guild_name + "** because **" + std::to_string(chrono::duration_cast<chrono::days>(member_for).count()) + "** days have passed since you joined and you didn't get a proficiency rank.\n"
						            "\n"
						            "But don't fret! You are free to rejoin, just make sure to:\n"
						            "- Verify your phone number\n"
						            "- Get a proficiency rank as mentioned in `#welcoming`\n"
						            "https://discord.gg/greek"
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
				cUtils::PrintLog("%s %s#%s Member since: %s Role count: %d", user->GetId().ToString(), user->GetUsername(), user->GetDiscriminator(), member.GetMemberSince(), (int) member.Roles.size());
			}
		}
		co_return co_await EditInteractionResponse(i, kw::content=cUtils::Format("Pruned **%d** member%s", total, total == 1 ? "" : "s"));
	}
	catch (const std::exception& e) {
		cUtils::PrintErr(e.what());
	}
	catch (...) {}
	co_await EditInteractionResponse(i, kw::content="An unexpected error has occurred, try again later.");
}