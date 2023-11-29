#include "GreekBot.h"

cTask<>
cGreekBot::process_dismiss(cMsgCompInteraction& i, const cSnowflake& user_id) HANDLER_BEGIN {
	/* If we're on a guild, check if the user is the original author or if they have appropriate permissions */
	if (i.GetUser().GetId() != user_id && i.GetMember() && !(i.GetMember()->GetPermissions() & PERM_MANAGE_MESSAGES))
		co_return co_await InteractionSendMessage(i, cMessageParams{
			kw::flags=MESSAGE_FLAG_EPHEMERAL,
			kw::content="You can't do that, you're missing the `MANAGE_MESSAGES` permission."
		});
	/* If all good, delete original message */
	co_await InteractionDefer(i);
	co_await InteractionDeleteMessage(i, i.GetMessage());
} HANDLER_END