#include "GreekBot.h"

cTask<>
cGreekBot::OnInteraction_ban(const cInteraction& i) {
	/* Acknowledge the interaction first */
	co_await AcknowledgeInteraction(i);
	/* Collect interaction options */
	try {
		if (chSnowflake pGuildId = i.GetGuildId(); pGuildId) {
			/* Parse options */
			chUser user = nullptr;
			chrono::seconds delete_messages = chrono::days(7);
			std::string reason = "Unspecified";
			auto &options = i.GetData<INTERACTION_APPLICATION_COMMAND>()->Options;
			for (auto& opt : options) {
				if (0 == strcmp(opt.GetName(), "user"))
					user = opt.GetValue<APP_COMMAND_OPT_USER>();
				else if (0 == strcmp(opt.GetName(), "delete")) {
					/* Convert the received value to int */
					int value = cUtils::ParseInt(options[1].GetValue<APP_COMMAND_OPT_STRING>());
					/* Deduce delete messages duration */
					switch (value) {
						case 0:
							delete_messages = chrono::hours(1);
							break;
						case 1:
							delete_messages = chrono::days(1);
							break;
						default:
							break;
					}
				}
				else if (0 == strcmp(opt.GetName(), "reason"))
					reason = opt.GetValue<APP_COMMAND_OPT_STRING>();
			}
			/* Making sure we're not banning ourselves */
			if (user->GetId() == GetUser()->GetId())
				co_return co_await EditInteractionResponse(i, MESSAGE_FLAG_NONE, { .content = "Pfffff, as if I'm gonna ban myself. Shame!" });

			co_await CreateDMMessage(user->GetId(), MESSAGE_FLAG_NONE, {
				.content = cUtils::Format("You've been banned from **%s** with reason:\n```%s```", m_guilds.at(*pGuildId)->GetName(), reason)
			});
			co_await CreateGuildBan(*pGuildId, user->GetId(), delete_messages, reason);

			/* TODO: std::string for cButton, like plz */
			std::string custom_id = cUtils::Format("ban_%s", user->GetId().ToString());
			co_await EditInteractionResponse(i, MESSAGE_FLAG_NONE, {
				.content = "Soon:tm:",
				.components = {
					cActionRow {
						cButton<BUTTON_STYLE_DANGER> {
							custom_id.c_str(),
							"Revoke ban"
						}
					}
				},
				.embeds = {
					cEmbed::CreateBuilder().SetAuthor(cUtils::Format("%s#%s was banned", user->GetUsername(), user->GetDiscriminator()), {}, user->GetAvatarUrl()).AddField("Reason", reason).Build()
				}
			});
			co_return;
		}
	}
	catch (...) {}
	co_await EditInteractionResponse(i, MESSAGE_FLAG_NONE, { .content = "An unexpected error has occurred, please try again later." });
}