#include "GreekBot.h"
#include "CDN.h"

cTask<>
cGreekBot::process_avatar(cAppCmdInteraction& i) HANDLER_BEGIN {
	/* Resolve user option */
	hUser user;
	if (auto options = i.GetOptions(); !options.empty())
		user = std::get<0>(options.front().GetValue<APP_CMD_OPT_USER>());
	else
		user = &i.GetUser();
	/* Respond */
	co_await InteractionSendMessage(i, cMessageParams{
		kw::content=cCDN::GetUserAvatar(*user)
	});
} HANDLER_END