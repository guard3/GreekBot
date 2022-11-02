#include "GreekBot.h"

cTask<>
cGreekBot::OnInteraction_dismiss(const cInteraction& i, const cSnowflake& user_id) {
	/* Acknowledge interaction first */
	co_await RespondToInteraction(i);
	try {
		if (chUser user = i.GetUser(); !user) {
			/* If we're on a guild, collect member and user info */
			chMember member = i.GetMember();
			if (!member) throw 0;
			if (!(user = member->GetUser())) throw 0;
			/* If current user is not the original author, check for appropriate permissions */
			if (user->GetId() != user_id) {
				if (!(member->GetPermissions() & PERM_MANAGE_MESSAGES))
					co_return co_await SendInteractionFollowupMessage(i, flags=MESSAGE_FLAG_EPHEMERAL, content="You can't do that, you're missing the `MANAGE_MESSAGES` permission.");
			}
		}
		/* If all's good, delete the original response */
		co_return co_await DeleteInteractionResponse(i);
	}
	catch (...) {}
	co_await SendInteractionFollowupMessage(i, flags=MESSAGE_FLAG_EPHEMERAL, content="An unexpected error has occurred. Try again later.");
}