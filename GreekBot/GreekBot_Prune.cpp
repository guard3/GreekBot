#include "GreekBot.h"

cTask<>
cGreekBot::OnInteraction_prune(const cInteraction& i) {
	/* If we're not on a guild, just do nothing */
	chSnowflake pGuildId = nullptr;
	if (pGuildId = i.GetGuildId(); !pGuildId)
		co_return co_await RespondToInteraction(i, flags=MESSAGE_FLAG_EPHEMERAL, content="Sorry, I can't do that in DMs or Group chats.");

	co_await AcknowledgeInteraction(i);
	int days = 2;
	auto& options = i.GetData<INTERACTION_APPLICATION_COMMAND>()->Options;
	if (!options.empty())
		days = options[0].GetValue<APP_CMD_OPT_INTEGER>();

	chrono::milliseconds retry_after;
	try {
		int pruned = co_await BeginGuildPrune(*pGuildId, days, "Failed to get a rank");
		co_return co_await EditInteractionResponse(i, content = cUtils::Format("Pruned **%d** member%s for **%d** day%s of inactivity.", pruned, pruned == 1 ? "" : "s", days, days == 1 ? "" : "s"));
	}
	catch (const xRateLimitError& e) {
		retry_after = e.retry_after();
	}
	co_await EditInteractionResponse(i, content = cUtils::Format("Rate limited. Try again after **%dms**.", (int)retry_after.count()));
}