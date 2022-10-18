#include "GreekBot.h"

cTask<>
cGreekBot::OnInteraction_ban(const cInteraction& i, const char* image_url) {
	/* If an image url is specified, send troll message */
	if (image_url) {
		cUtils::PrintLog("Hihi, troll about to be trolled");
		co_await RespondToInteraction(i, MESSAGE_FLAG_NONE, {
			.clear_components = true
		});
		co_await SendInteractionFollowupMessage(i, MESSAGE_FLAG_NONE, {.content=image_url});
		co_return;
	}
	/* Acknowledge the interaction first */
	co_await AcknowledgeInteraction(i);
	/* Collect interaction data */
	chSnowflake guild_id = i.GetGuildId();
	chMember    member   = i.GetMember();
	if (guild_id && member) {
		/* Check that the invoking user has appropriate permissions, for extra measure */
		if (!(member->GetPermissions() & PERM_BAN_MEMBERS))
			co_return co_await EditInteractionResponse(i, MESSAGE_FLAG_NONE, {.content="You can't do that. You're missing the `BAN_MEMBERS` permission."});
		try {
			/* Check which subcommand was invoked */
			auto& subcommand_option = i.GetData<INTERACTION_APPLICATION_COMMAND>()->Options.front();
			auto& options = subcommand_option.GetOptions();
			if (subcommand_option.GetName() == "turk") {
				/* Get the troll user to be banned */
				auto& user = options.front().GetValue<APP_CMD_OPT_USER>();
				/* Make sure we're not banning ourselves */
				if (user.GetId() == GetUser()->GetId())
					co_return co_await EditInteractionResponse(i, MESSAGE_FLAG_NONE, {.content = "Excuse me, ı'm not türk! Beep bop... 🤖"});
				/* TODO: choose a random goodbye message */
				const char *msg = "senin ananı hacı bekir efendinin şalvarına dolar yan yatırır sokağındaki caminin minaresinde öyle bir sikerim ki ezan okundu sanırsın sonra minareden indirir koca yarrağımla köyünü yağmalar herkesi ortadox yaparım seni veled-i zina seni dljs;fjaiaejadklsjkfjdsjfklsdjflkjds;afdkslfdksfdlsfs";
				/* DM the goodbye message */
				co_await CreateDMMessage(user.GetId(), MESSAGE_FLAG_NONE, {
					.content = cUtils::Format("You've been banned from **%s** with reason:\n```%s```Do you want to appeal the ban?", m_guilds.at(*guild_id)->GetName(), msg),
					.components = {
						cActionRow {
							cButton<BUTTON_STYLE_SUCCESS> {
								STR(CMP_ID_BUTTON_TURK_A),
								"Evet"
							},
							cButton<BUTTON_STYLE_DANGER> {
								STR(CMP_ID_BUTTON_TURK_B),
								"Hayır"
							}
						}
					}
				});
				/* Create the ban */
				co_await CreateGuildBan(*guild_id, user.GetId(), chrono::days(7), "Turk troll");
				/* Send confirmation message */
				co_await EditInteractionResponse(i, MESSAGE_FLAG_NONE, {
					.embeds = {{
						author = {
							cUtils::Format("%s#%s was banned", user.GetUsername(), user.GetDiscriminator()),
							icon_url = user.GetAvatarUrl()
						},
						fields = {
							{"Reason", "Turk troll"},
							{"Goodbye message", msg}
						}
				   }}
				});
				co_return;
			}
			/* Handle ban for regular user */
			chUser user = nullptr;
			chrono::seconds delete_messages = chrono::days(7);
			std::string reason = "Unspecified";
			for (auto &opt: options) {
				if (opt.GetName() == "user")
					user = &opt.GetValue<APP_CMD_OPT_USER>();
				else if (opt.GetName() == "delete") {
					/* Convert the received value to int */
					int value = cUtils::ParseInt(opt.GetValue<APP_CMD_OPT_STRING>());
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
				else if (opt.GetName() == "reason")
					reason = opt.GetValue<APP_CMD_OPT_STRING>();
			}
			/* Make sure a user is specified */
			if (!user) throw std::exception();
			/* Make sure we're not banning ourselves */
			if (user->GetId() == GetUser()->GetId())
				co_return co_await EditInteractionResponse(i, MESSAGE_FLAG_NONE, {.content = "I'm not gonna ban myself, I'm a good bot. Beep bop... 🤖"});
			/* DM the ban reason */
			co_await CreateDMMessage(user->GetId(), MESSAGE_FLAG_NONE, {
				.content = cUtils::Format("You've been banned from **%s** with reason:\n```%s```", m_guilds.at(*guild_id)->GetName(), reason)
			});
			/* Create the ban; TODO: convert 'reason' string to be url-encoded */
			co_await CreateGuildBan(*guild_id, user->GetId(), delete_messages, reason);
			/* Send confirmation message */
			co_await EditInteractionResponse(i, MESSAGE_FLAG_NONE, {
				.components = {
					cActionRow {
						cButton<BUTTON_STYLE_DANGER>{
							cUtils::Format("BAN#%s", user->GetId().ToString()).c_str(), // TODO: std::string for cButton, like plz
							"Revoke ban"
						}
					}
				},
				.embeds = {{
					author={
						cUtils::Format("%s#%s was banned", user->GetUsername(), user->GetDiscriminator()),
						icon_url = user->GetAvatarUrl()
					},
					fields={{"Reason", reason}}
				}}
			});
			co_return;
		}
		catch (...) {}
	}
	co_await EditInteractionResponse(i, MESSAGE_FLAG_NONE, { .content = "An unexpected error has occurred, please try again later." });
#if 0
	/* Collect interaction options */
	try {
		if (chSnowflake pGuildId = i.GetGuildId(); pGuildId) {
			/* Parse options */
			chUser user = nullptr;
			chrono::seconds delete_messages = chrono::days(7);
			std::string reason = "Unspecified";
			auto &options = i.GetData<INTERACTION_APPLICATION_COMMAND>()->Options;
			for (auto& opt : options) {
				if (opt.GetName() == "user")
					user = &opt.GetValue<APP_CMD_OPT_USER>();
				else if (opt.GetName() == "delete") {
					/* Convert the received value to int */
					int value = cUtils::ParseInt(opt.GetValue<APP_CMD_OPT_STRING>());
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
				else if (opt.GetName() == "reason")
					reason = opt.GetValue<APP_CMD_OPT_STRING>();
			}
			/* Making sure we're not banning ourselves */
			if (user->GetId() == GetUser()->GetId())
				co_return co_await EditInteractionResponse(i, MESSAGE_FLAG_NONE, { .content = "Pfffff, as if I'm gonna ban myself. Shame!" });

			co_await CreateDMMessage(user->GetId(), MESSAGE_FLAG_NONE, {
				.content = cUtils::Format("You've been banned from **%s** with reason:\n```%s```", m_guilds.at(*pGuildId)->GetName(), reason)
			});
			co_await CreateGuildBan(*pGuildId, user->GetId(), delete_messages, reason);

			/* TODO: std::string for cButton, like plz */
			uint64_t user_id_int = user->GetId().ToInt();
			std::string custom_id = "BAN#" + cUtils::Base64Encode(&user_id_int, sizeof(user_id_int));
			co_await EditInteractionResponse(i, MESSAGE_FLAG_NONE, {
				.components = {
					cActionRow {
						cButton<BUTTON_STYLE_DANGER> {
							custom_id.c_str(),
							"Revoke ban"
						}
					}
				},
				.embeds = {{
					author={
						cUtils::Format("%s#%s was banned", user->GetUsername(), user->GetDiscriminator()),
						icon_url=user->GetAvatarUrl()
					},
					fields={{"Reason", reason}}
				}}
			});
			co_return;
		}
	}
	catch (...) {}
	co_await EditInteractionResponse(i, MESSAGE_FLAG_NONE, { .content = "An unexpected error has occurred, please try again later." });
#endif
}

cTask<>
cGreekBot::OnInteraction_unban(const cInteraction& i, const cSnowflake& user_id) {
	if (chMember member = i.GetMember()) {
		/* Make sure that the invoking user has the appropriate permissions */
		if (!(member->GetPermissions() & PERM_BAN_MEMBERS)) {
			co_await RespondToInteraction(i, MESSAGE_FLAG_NONE);
			co_return co_await SendInteractionFollowupMessage(i, MESSAGE_FLAG_EPHEMERAL, {
				.content = "You can't do that. You're missing the `BAN_MEMBERS` permission."
			});
		}
		if (chSnowflake pGuildId = i.GetGuildId()) {
			co_await AcknowledgeInteraction(i);
			try {
				co_await RemoveGuildBan(*pGuildId, user_id);
			}
			catch (xDiscordError& e) {
				/* Ban not found, but that's fine */
			}
			auto e = i.GetMessage()->Embeds[0];
			auto& s = e.GetAuthor()->GetName();
			s.erase(s.end()-11, s.end());
			e.SetFields(nullptr).SetDescription("User was unbanned");
			co_await EditInteractionResponse(i, MESSAGE_FLAG_NONE, {
				.clear_components = true,
				.embeds { std::move(e) }
			});
			co_return;
		}
	}
}