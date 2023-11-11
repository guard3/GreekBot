#include "GreekBot.h"

cTask<>
cGreekBot::process_dismiss(cMsgCompInteraction& i, const cSnowflake& user_id) {
	/* Acknowledge interaction first */
	co_await RespondToInteraction(i);
	try {
		/* If we're on a guild, check if the user is the original author or if they have appropriate permissions */
		if (i.GetGuildId()) {
			if (i.GetUser().GetId() != user_id && !(i.GetMember()->GetPermissions() & PERM_MANAGE_MESSAGES))
				co_return co_await SendInteractionFollowupMessage(i, kw::flags=MESSAGE_FLAG_EPHEMERAL, kw::content="You can't do that, you're missing the `MANAGE_MESSAGES` permission.");
		}
		/* If all's good, delete the original response */
		co_return co_await DeleteInteractionResponse(i);
	}
	catch (...) {}
	co_await SendInteractionFollowupMessage(i, kw::flags=MESSAGE_FLAG_EPHEMERAL, kw::content="An unexpected error has occurred. Try again later.");
}