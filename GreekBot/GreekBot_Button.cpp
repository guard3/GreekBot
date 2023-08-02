#include "GreekBot.h"

cTask<>
cGreekBot::OnInteraction_button(const cInteraction& i) {
	co_await RespondToInteraction(i);
	co_await SendInteractionFollowupMessage(i, kw::flags=MESSAGE_FLAG_EPHEMERAL, kw::content="Every minute that you're messaging, you randomly gain between 15 and 25 **XP**.");
}