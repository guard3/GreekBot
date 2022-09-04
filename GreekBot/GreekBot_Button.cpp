#include "GreekBot.h"

cTask<>
cGreekBot::OnInteraction_button(const cInteraction& i) {
	try {
		auto embeds = i.GetMessage()->Embeds;
		cEmbed e = cEmbed::CreateBuilder()
			.SetColor(embeds.back().GetColor())
			.SetTitle("How it works")
			.SetDescription("Every minute that you're messaging, you randomly gain between 15 and 25 **XP**.")
			.Build();
		if (embeds.size() == 10)
			embeds[9] = std::move(e);
		else
			embeds.push_back(std::move(e));
		co_await RespondToInteraction(i, MESSAGE_FLAG_NONE, {.embeds = std::move(embeds)});
	}
	catch (const std::exception& e) {
		cUtils::PrintErr("OnInteraction_button: %s", e.what());
	}
}