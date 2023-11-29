#include "GreekBot.h"
#include <fmt/chrono.h>

cTask<>
cGreekBot::process_prune(cAppCmdInteraction& i) HANDLER_BEGIN {
	/* Make sure that the user has the necessary permissions */
	if (!(i.GetMember()->GetPermissions() & PERM_KICK_MEMBERS))
		co_return co_await InteractionSendMessage(i, cMessageParams{
			kw::flags=MESSAGE_FLAG_EPHEMERAL,
			kw::content="You can't do that. You're missing the `KICK_MEMBERS` permission."
		});
	/* How many days of inactivity to consider */
	int days = i.GetOptions().empty() ? 2 : i.GetOptions().front().GetValue<APP_CMD_OPT_INTEGER>();
	/* Prune */
	co_await InteractionDefer(i);
	std::string str;
	try {
		int pruned = co_await BeginGuildPrune(*i.GetGuildId(), days, "Failed to get a rank");
		str = fmt::format("Pruned **{}** member{} for **{}** day{} of inactivity.", pruned, pruned == 1 ? "" : "s", days, days == 1 ? "" : "s");
	}
	catch (const xRateLimitError& e) {
		str = fmt::format("Rate limited. Try again after **{}**.", e.retry_after());
	}
	/* Send confirmation message */
	co_await InteractionSendMessage(i, cMessageParams{
		kw::content=std::move(str),
		kw::components={
			cActionRow{
				cButton{
					BUTTON_STYLE_SECONDARY,
					fmt::format("DLT#{}", i.GetUser().GetId()),
					kw::label = "Dismiss"
				}
			}
		}
	});
} HANDLER_END

cTask<>
cGreekBot::process_prune_lmg(cAppCmdInteraction& i) HANDLER_BEGIN {
	using namespace std::chrono;
	using namespace std::chrono_literals;
	/* Check that the invoking member has the appropriate permissions for extra security measure */
	if (!(i.GetMember()->GetPermissions() & PERM_KICK_MEMBERS))
		co_return co_await InteractionSendMessage(i, cMessageParams{
			kw::flags=MESSAGE_FLAG_EPHEMERAL,
			kw::content="You can't do that. You're missing the `KICK_MEMBERS` permission."
		});
	/* Otherwise, carry on normally */
	co_await InteractionDefer(i);
	auto before = system_clock::now();
	/* Collect guild information */
	const cGuild& guild = *m_guilds[*i.GetGuildId()];
	cSnowflake guild_id = guild.GetId();
	std::string guild_name = guild.GetName();
	int total = 0, fails = 0;
	/* Get all members of the guild */
	for (auto gen = GetGuildMembers(guild_id); co_await gen.HasValue();) {
		cMember member = co_await gen();
		/* Interaction tokens are valid for 15 minutes, so, just to be safe, abort after 14 minutes have passed*/
		auto now = system_clock::now();
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
						guild_name, duration_cast<days>(member_for).count())
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
	co_await InteractionSendMessage(i, cMessageParams{
		kw::content=fmt::format("Pruned **{}** member{}", total, total == 1 ? "" : "s"),
		kw::components={
			cActionRow{
				cButton{
					BUTTON_STYLE_SECONDARY,
					fmt::format("DLT#{}", i.GetUser().GetId()),
					kw::label="Dismiss"
				}
			}
		}
	});
} HANDLER_END