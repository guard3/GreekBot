#include "GreekBot.h"

cTask<>
cGreekBot::OnInteraction_button(const cInteraction& i) {
	try {
		auto es = i.GetMessage()->Embeds;
		cEmbed e {
			color=es.back().GetColor(),
			title="How it works",
			description="Every minute that you're messaging, you randomly gain between 15 and 25 **XP**."
		};
		if (es.size() == 10)
			es[9] = std::move(e);
		else
			es.push_back(std::move(e));
		co_await RespondToInteraction(i, components=nullptr, embeds=std::move(es));
	}
	catch (const std::exception& e) {
		cUtils::PrintErr("OnInteraction_button: %s", e.what());
	}
}