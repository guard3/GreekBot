#include "GreekBot.h"
#include "Utils.h"

cTask<>
cGreekBot::process_avatar(cAppCmdInteraction& i) {
	/* Resolve user option */
	hUser user;
	if (auto options = i.GetOptions(); !options.empty())
		user = &options.front().GetValue<APP_CMD_OPT_USER>();
	else
		user = i.GetUser() ? i.GetUser() : i.GetMember()->GetUser();
	/* Respond */
	co_await RespondToInteraction(i, kw::content=user->MoveAvatarUrl());
}