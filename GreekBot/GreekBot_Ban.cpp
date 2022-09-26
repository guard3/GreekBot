#include "GreekBot.h"

cTask<>
cGreekBot::OnInteraction_ban(const cInteraction& i) {
	co_await AcknowledgeInteraction(i);
	/* Collect interaction options */
	chSnowflake guild_id = i.GetGuildId();
	if (!guild_id)
		co_return;

	auto& options = i.GetData<INTERACTION_APPLICATION_COMMAND>()->Options;
	chUser user = options[0].GetValue<APP_COMMAND_OPT_USER>();
	int delete_seconds = 604800; // 7 days by default
	if (options.size() > 1) {
		/* Convert the received value to int */
		char* end;
		const char* s = options[1].GetValue<APP_COMMAND_OPT_STRING>();
		unsigned long long value = strtoull(s, &end, 10);
		if (*end)
			co_return;
		/* Deduce delete messages duration */
		switch (value) {
			case 0:
				delete_seconds = 3600; // 1 hour
				break;
			case 1:
				delete_seconds = 24 * 3600; // 24 hours
				break;
			default:
				break;
		}
	}
	std::string reason = options.size() > 2 ? options[2].GetValue<APP_COMMAND_OPT_STRING>() : "Unspecified";

	/* Acknowledge the interaction first */

	co_await CreateDMMessage(user->GetId(), MESSAGE_FLAG_NONE, {
		.content = cUtils::Format("You've been banned from *guild name tba* with reason:\n```%s```", reason)
	});
	co_await EditInteractionResponse(i, MESSAGE_FLAG_NONE, {.content = "Soon:tm:"});

}