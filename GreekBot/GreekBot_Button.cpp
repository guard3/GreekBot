#include "GreekBot.h"

void
cGreekBot::OnInteraction_button(chInteraction interaction) {
	auto embeds = interaction->GetMessage()->Embeds;
	embeds.push_back(
		cEmbed::CreateBuilder()
		.SetColor(embeds[0].GetColor())
		.SetTitle("How it works")
		.SetDescription("Every minute that you're messaging, you randomly gain between 15 and 25 **XP**.")
		.Build()
	);
	RespondToInteraction(interaction, nullptr, MESSAGE_FLAG_NONE, embeds, nullptr, std::vector<cActionRow>(), nullptr);
}