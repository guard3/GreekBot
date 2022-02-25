#include "GreekBot.h"

void
cGreekBot::OnInteraction_avatar(chInteraction interaction) {
	/* Get interaction data */
	auto data = interaction->GetData<INTERACTION_APPLICATION_COMMAND>();
	/* Resolve user option */
	chUser user;
	if (data->Options.empty())
		user = interaction->GetUser() ? interaction->GetUser() : interaction->GetMember()->GetUser();
	else
		user = data->Options[0].GetValue<APP_COMMAND_OPT_USER>();
	/* Respond */
	RespondToInteraction(interaction, user->GetAvatarUrl().c_str(), MESSAGE_FLAG_NONE, nullptr, nullptr, nullptr, nullptr);
}
