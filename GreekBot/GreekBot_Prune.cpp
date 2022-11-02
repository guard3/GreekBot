#include "GreekBot.h"

cTask<>
cGreekBot::OnInteraction_prune(const cInteraction& i) {
	/* Acknowledge interaction first */
	co_await AcknowledgeInteraction(i);
	/* Content of the message in case of an error */
	//std::string str;
	//bool bDismissButton = false;
	try {
		/* Make sure that we're on a guild and that we have the necessary permissions */
		chSnowflake guild_id = i.GetGuildId();
		chMember member = i.GetMember();
		if (!guild_id || !member) throw 0;
		if (!(member->GetPermissions() & PERM_KICK_MEMBERS))
			co_return co_await EditInteractionResponse(i, content="You can't do that. You're missing the `KICK_MEMBERS` permission.");
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
			content=std::move(str),
			components = {
				cActionRow{
					cButton<BUTTON_STYLE_SECONDARY>{
						cUtils::Format("DLT#%s", member->GetUser()->GetId().ToString()),
						label = "Dismiss"
					}
				}
			}
		);
	}
	catch (...) {}
	co_await EditInteractionResponse(i, content="An unexpected error has occurred, try again later.");
}