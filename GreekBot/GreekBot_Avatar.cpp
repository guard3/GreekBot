#include "GreekBot.h"

cTask<>
cGreekBot::OnInteraction_avatar(const cInteraction& i) try {
	/* Get interaction data */
	auto data = i.GetData<INTERACTION_APPLICATION_COMMAND>();
	/* Resolve user option */
	chUser user;
	if (data->Options.empty())
		user = i.GetUser() ? i.GetUser() : i.GetMember()->GetUser();
	else
		user = &data->Options.front().GetValue<APP_CMD_OPT_USER>();
	/* Respond */
	co_await RespondToInteraction(i, content=user->GetAvatarUrl());
}
catch (const std::exception& e) {
	cUtils::PrintErr("OnInteraction_avatar: %s", e.what());
}