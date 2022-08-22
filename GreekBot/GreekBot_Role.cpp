#include "GreekBot.h"

void
cGreekBot::OnInteraction_role(chInteraction interaction) {
	try {
		RespondToInteraction(
			interaction,
			"Select a role depending on your greek level:",
			MESSAGE_FLAG_EPHEMERAL,
			nullptr,
			nullptr,
			std::vector<cActionRow>{
				cActionRow{
					cSelectMenu{
						"proficiency_role_menu",
						"Choose an option...",
						cSelectOption{
							"Native",
							"opt_gr",
							"If greek is your native language",
							cEmoji("level_gr", "875469185529036880")
						},
						cSelectOption{
							"Beginner",
							"opt_a1",
							"If you just started learning greek or your level is A1",
							cEmoji("level_a1", "875469185793286164")
						},
						cSelectOption{
							"Elementary",
							"opt_a2",
							"If your greek level is A2",
							cEmoji("level_a2", "875469185394827355")
						},
						cSelectOption{
							"Intermediate",
							"opt_b1",
							"If your greek level is B1",
							cEmoji("level_b1", "875469185659056138")
						},
						cSelectOption{
							"Upper Intermediate",
							"opt_b2",
							"If your greek level is B2",
							cEmoji("level_b2", "875469185751347251")
						},
						cSelectOption{
							"Advanced",
							"opt_c1",
							"If your greek level is C1",
							cEmoji("level_c1", "875469185726173276")
						},
						cSelectOption{
							"Fluent",
							"opt_c2",
							"If your greek level is C2",
							cEmoji("level_c2", "875469185734541382")
						},
						cSelectOption{
							"Non Learner",
							"opt_no",
							"If you don't want to learn greek",
							cEmoji("level_no", "875469185466109992")
						}
					}
				},
				cActionRow{
					cButton<BUTTON_STYLE_LINK>{
						"https://en.wikipedia.org/wiki/Common_European_Framework_of_Reference_for_Languages",
						"Don't know what to pick?"
					}
				}
			},
			nullptr
		);
	}
	catch (const std::exception& e) {
		cUtils::PrintErr("OnInteraction_role: %s", e.what());
	}
}
