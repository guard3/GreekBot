#include "GreekBot.h"

void
cGreekBot::OnInteraction_button(chInteraction interaction) {
	auto embeds = interaction->GetMessage()->Embeds;
	cEmbed e = cEmbed::CreateBuilder()
		.SetColor(embeds[0].GetColor())
		.SetTitle("How it works")
		.SetDescription("Every minute that you're messaging, you randomly gain between 15 and 25 **XP**.")
		.Build();
	if (embeds.size() == 10)
		embeds[9] = std::move(e);
	else
		embeds.push_back(std::move(e));
	RespondToInteraction(interaction, nullptr, MESSAGE_FLAG_NONE, embeds, nullptr, std::vector<cActionRow>(), nullptr);
}