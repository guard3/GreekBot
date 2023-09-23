#include "GreekBot.h"
#include "Utils.h"

cTask<>
cGreekBot::OnInteraction_role(const cInteraction& interaction) try {
	co_await RespondToInteraction(
		interaction,
		kw::flags=MESSAGE_FLAG_EPHEMERAL,
		kw::content="Select a role depending on your greek level:",
		kw::components={
			cActionRow {
				cSelectMenu {
					"proficiency_role_menu",
					{
						{
							"Native",
							"opt_gr",
							kw::description = "If greek is your native language",
							kw::emoji = cEmoji("level_gr", "875469185529036880")
						}, {
							"Beginner",
							"opt_a1",
							kw::description = "If you just started learning greek or your level is A1",
							kw::emoji = cEmoji("level_a1", "875469185793286164")
						}, {
							"Elementary",
							"opt_a2",
							kw::description = "If your greek level is A2",
							kw::emoji = cEmoji("level_a2", "875469185394827355")
						}, {
							"Intermediate",
							"opt_b1",
							kw::description = "If your greek level is B1",
							kw::emoji = cEmoji("level_b1", "875469185659056138")
						}, {
							"Upper Intermediate",
							"opt_b2",
							kw::description = "If your greek level is B2",
							kw::emoji = cEmoji("level_b2", "875469185751347251")
						}, {
							"Advanced",
							"opt_c1",
							kw::description = "If your greek level is C1",
							kw::emoji = cEmoji("level_c1", "875469185726173276")
						}, {
							"Fluent",
							"opt_c2",
							kw::description = "If your greek level is C2",
							kw::emoji = cEmoji("level_c2", "875469185734541382")
						}, {
							"Non Learner",
							"opt_no",
							kw::description = "If you don't want to learn greek",
							kw::emoji = cEmoji("level_no", "875469185466109992")
						}
					},
					kw::placeholder="Choose an option..."
				}
			},
			cActionRow{
				cLinkButton{
					"https://en.wikipedia.org/wiki/Common_European_Framework_of_Reference_for_Languages",
					kw::label="Don't know what to pick?"
				}
			}
		}
	);
}
catch (const std::exception& e) {
	cUtils::PrintErr("OnInteraction_role: {}", e.what());
}
