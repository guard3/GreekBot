#include "GreekBot.h"

cTask<>
cGreekBot::OnInteraction_button(const cInteraction& i) {
	try {
		auto embeds = i.GetMessage()->Embeds;
		cEmbed e {
			color=embeds.back().GetColor(),
			title="How it works",
			description="Every minute that you're messaging, you randomly gain between 15 and 25 **XP**."
		};
		if (embeds.size() == 10)
			embeds[9] = std::move(e);
		else
			embeds.push_back(std::move(e));
		co_await RespondToInteraction(i, MESSAGE_FLAG_NONE, {.clear_components = true, .embeds = std::move(embeds)});
	}
	catch (const std::exception& e) {
		cUtils::PrintErr("OnInteraction_button: %s", e.what());
	}
}