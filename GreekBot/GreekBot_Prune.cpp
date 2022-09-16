#include "GreekBot.h"

cTask<>
cGreekBot::OnInteraction_prune(const cInteraction& i) {
	/* If we're not on a guild, just do nothing */
	chSnowflake pGuildId = nullptr;
	if (pGuildId = i.GetGuildId(); !pGuildId)
		co_return co_await RespondToInteraction(i, MESSAGE_FLAG_EPHEMERAL, { .content = "Sorry, I can't do that in DMs or Group chats." });

	co_await AcknowledgeInteraction(i);
	int days = 2;
	auto& options = i.GetData<INTERACTION_APPLICATION_COMMAND>()->Options;
	if (!options.empty())
		days = options[0].GetValue<APP_COMMAND_OPT_INTEGER>();

	int pruned = co_await BeginGuildPrune(*pGuildId, days, "Failed to get a rank");
	co_await EditInteractionResponse(i, MESSAGE_FLAG_EPHEMERAL, { .content = cUtils::Format("Pruned **%d** member%s for **%d** days of inactivity.", pruned, pruned == 1 ? "" : "s", days) });
}