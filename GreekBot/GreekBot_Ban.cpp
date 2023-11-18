#include "GreekBot.h"
#include "Utils.h"
#include <fmt/format.h>

// TODO: simplify everything more

static std::string get_no_ban_msg(eSubcommand e) {
	const char* msg;
	switch (e) {
		case SUBCMD_TURK:
			msg = "Excuse me, ı'm not türk!";
			break;
		case SUBCMD_GREEK:
			msg = "Όπα κάτσε, τι έκανα; Είμαι καλό μποτ εγώ.";
			break;
		default:
			msg = "I'm not gonna ban myself, I'm a good bot.";
			break;
	}
	return fmt::format("{} Beep Bop...🤖", msg);
}

cTask<>
cGreekBot::process_ban(cAppCmdInteraction& i) {
	using namespace std::chrono;
	/* Acknowledge the interaction first */
	co_await RespondToInteraction(i);
	try {
		/* Collect interaction data */
		const cSnowflake& guild_id = *i.GetGuildId();
		/* Check that the invoking user has appropriate permissions, for extra measure */
		if (!(i.GetMember()->GetPermissions() & PERM_BAN_MEMBERS))
			co_return co_await EditInteractionResponse(i, kw::content="You can't do that. You're missing the `BAN_MEMBERS` permission.");
		/* Get the subcommand and its options */
		auto& subcommand = i.GetOptions().front();
		uint32_t subcmd = cUtils::CRC32(0, subcommand.GetName());
		/* Collect options */
		hUser user;
		seconds delete_messages = days(7);
		std::string_view reason, goodbye;
		for (auto& opt: subcommand.GetOptions()) {
			switch (cUtils::CRC32(0, opt.GetName())) {
				case 0x8D93D649: // "user"
					user = std::get<0>(opt.GetValue<APP_CMD_OPT_USER>());
					break;
				case 0x3A127C87: // "delete"
					delete_messages = hours(cUtils::ParseInt(opt.GetValue<APP_CMD_OPT_STRING>()));
					break;
				case 0x3BB8880C: // "reason"
					reason = opt.GetValue<APP_CMD_OPT_STRING>();
					break;
				case 0xB6BD307F: // "message"
					goodbye = opt.GetValue<APP_CMD_OPT_STRING>();
				default:
					break;
			}
		}
		/* Make sure we're not banning ourselves */
		if (user->GetId() == GetUser()->GetId())
			co_return co_await EditInteractionResponse(i, kw::content=get_no_ban_msg((eSubcommand)subcmd));
		/* Ban */
		cMessageParams msg = co_await process_ban(i, subcmd, *user, delete_messages, reason, goodbye);
		co_return co_await EditInteractionResponse(i, msg);
	}
	catch (...) {}
	co_await EditInteractionResponse(i, kw::content="An unexpected error has occurred, try again later.");
}

cTask<cMessageParams>
cGreekBot::process_ban(cInteraction& i, uint32_t subcmd, cUser& user, std::chrono::seconds delete_messages, std::string_view reason, std::string_view goodbye) {
	/* Update reason and goodbye message */
	switch (subcmd) {
		case SUBCMD_TURK:
			reason = "Turk troll";
			goodbye = "senin ananı hacı bekir efendinin şalvarına dolar yan yatırır sokağındaki caminin minaresinde öyle bir sikerim ki ezan okundu sanırsın sonra minareden indirir koca yarrağımla köyünü yağmalar herkesi ortadox yaparım seni veled-i zina seni dljs;fjaiaejadklsjkfjdsjfklsdjflkjds;afdkslfdksfdlsfs";
			break;
		case SUBCMD_GREEK: {
			static const std::string_view goodbyes[] {
				"Θα σε γαμήσω χιώτικα. Θα μυρίζει η σούφρα σου μαστίχα έναν χρόνο",
				"Αν η μαλακία ήταν γαρίφαλλο θα ήσουν επιτάφιος",
				"Δεν μπορώ να κλάσω αρκετά δυνατά για να σου απαντήσω όπως σου αρμόζει",
				"Ἔστιν οὖν τραγῳδία μίμησις πράξεως σπουδαίας καὶ τελείας, μέγεθος ἐχούσης, ἡδυσμένῳ λόγῳ, χωρὶς ἑκάστῳ τῶν εἰδῶν ἐν τοῖς μορίοις, δρώντων καὶ οὐ δι' ἀπαγγελίας, δι' ἐλέου καὶ φόβου περαίνουσα τὴν τῶν τοιούτων παθημάτων κάθαρσιν.",
				"Η αντιπρογιαγιά μου έκατσε στο σουλεϊμαν το μεγαλοπρεπή, ο προπάππους μου ήταν ο προσωπικός γιατρός του Κεμάλ Ατατούρκ, η προθεία μου η μεταφράστρια του πασά, είμαι ΑΕΚτζού χανούμισα, μου αρέσει το οθωμανικό, τα μουστάκια, οι φερετζέδες, τα belly dances, ο μπακλαβάς, το καρπούζι, το τζατζίκι, τα κεμπάπια, τα χαλιά, τα χάλια, η χλίδα, οι προστυχορυθμοί τους, τα τσιφτετέλια τους, το χαλούμι, το κανταΐφι, το ταου γιοξού, το τσανά καλέ, ο χαλβάς, ο καϊφές μερακλαντάν και σκέφτομαι να γίνω μουσουλμάνος, διότι κάνει καλό στη μέση.",
				"Άι μωρή μποχλάδω",
				"Την πέθανα, την σκότωσα την πουτάνα\n    - Ηλίας Ψινάκης",
				"Λοιπόν, ήρθε ο Πούρσας να με κάνει νταντά ντιέμ επειδή έκανα screenshot τα χόμο μηνύματα που μου έστελνε το Γατάκι. Όλο μιμς και πλακίτσα για άλλους αλλά μόλις σκάσει το ραντ πάνω τους το χιουμοράκι πάει περίπατο και αρχίζουν να κατεβάζουν μιουτς και μπανς. Και δώσε κλάμαααα οι σόυγιακς \"μου έκανε προσωπική επίθεση σνιφ σνιφ παλιοεντζλορντττ\". Μετά τις κωλοτούμπες στα γλωσσολογικά μόλις είπα ότι δεν γουστάρω ιμπεριαλιστές νεοθωμανούς γενοκτόνους εγκληματίες έβγαλες αιμορροίδες από το μπατχερτιλίκι και άρχισες τα καρενίστικα. Ούτε καν τους τύπους δεν κρατάς πλέον, το παίζεις και Πόντιος ξεφτιλισμένη ρεβιζιονιστική κατσαριδούλα. Φαίνεται από τα logs βέβαια πως τα ίδια κάνεις με όλους, μόλις κάποιος κάνει expose πόσο crackpots είστε. Αλλά τι λέμε τώρα, εδώ έχεις πει -χωρίς να ντρέπεσαι- πως \"δεν είναι δημοκρατικός ο σέρβερ\".",
				"Ρώτα τον wannabe σανταυμαρίτη καρπαζοεισπράκτορα να σου πει τι κάνουν οι πραγματικοί αριστεροί Σφακιώτες στο Σπανοχώρι σε πρακτορίσκους σαν εσένα. Δυο αργόσχολοι παρθένοι τσατάκηδες είστε εσύ και ο άλλος ο κλόουν που παπαγαλίζετε σαν στούρνοι ότι μαλακία δείτε στην lifo. Οπότε βούλωνε και μάθε τη θέση σου. Ο καλύτερος από εσάς το πολύ να κάνει κανένα ιδιαίτερο αύριο σε οθωμανικά mantrain για να ταϊσει την πουτάνα την μάνα του. Τώρα τράβα να με μπανάρεις παιδάκι και να μου κάνεις τα τρία δύο μην τυχόν και κόψει η κυκλομαλακία σας. Τουλάχιστον πλέον ξέρεις πως μόνο σε ανυποψίαστους ξένους περνάνε οι παπαριές σας."
			};
			reason = "Greek troll";
			goodbye = goodbyes[cUtils::Random(0u, std::size(goodbyes) - 1)];
			break;
		}
		default:
			if (reason.empty())
				reason = "Unspecified";
			if (goodbye.empty())
				goodbye = reason;
			break;
	}
	/* Create the embed of the confirmation message */
	std::vector<cEmbed> embeds;
	cEmbed& e = embeds.emplace_back(
		kw::author={
			fmt::format("{} was banned", user.GetUsername()),
			kw::icon_url=user.GetAvatarUrl()
		},
		kw::color=0xC43135,
		kw::fields={{ "Reason", reason }}
	);
	/* DM the goodbye message */
	try {
		co_await CreateDMMessage(
			user.GetId(),
			kw::content=fmt::format("You've been banned from **{}** with reason:\n```{}```", m_guilds.at(*i.GetGuildId())->GetName(), goodbye)
		);
		/* Add the goodbye message field only after the DM was sent successfully */
		if (reason != goodbye)
			e.AddField("Goodbye message", goodbye);
	}
	catch (...) {
		/* Couldn't send ban reason in DMs, the user may not be a member of the guild but that's fine */
	}
	/* Ban */
	co_await CreateGuildBan(*i.GetGuildId(), user.GetId(), delete_messages, reason);
	/* Send confirmation message */
	co_return cMessageParams{
		kw::components = {
			cActionRow{
				cButton{
					BUTTON_STYLE_DANGER,
					fmt::format("BAN#{}", user.GetId()),
					kw::label="Revoke ban"
				},
				cButton{
					BUTTON_STYLE_SECONDARY,
					fmt::format("DLT#{}", i.GetUser().GetId()),
					kw::label="Dismiss"
				}
			}
		},
		kw::embeds=std::move(embeds)
	};
}

cTask<>
cGreekBot::process_unban(cMsgCompInteraction& i, const cSnowflake& user_id) {
	/* Make sure that the invoking user has the appropriate permissions */
	if (!(i.GetMember()->GetPermissions() & PERM_BAN_MEMBERS)) {
		co_await RespondToInteraction(i);
		co_return co_await SendInteractionFollowupMessage(i, kw::flags=MESSAGE_FLAG_EPHEMERAL, kw::content="You can't do that. You're missing the `BAN_MEMBERS` permission.");
	}
	if (chSnowflake pGuildId = i.GetGuildId()) {
		co_await RespondToInteraction(i);
		try {
			co_await RemoveGuildBan(*pGuildId, user_id);
		}
		catch (xDiscordError& e) {
			/* Ban not found, but that's fine */
		}
		auto e = i.GetMessage().GetEmbeds().front();
		auto name = e.GetAuthor()->GetName();
		name.remove_suffix(11); // Remove the " was banned" part
		e.ClearFields().SetColor(0x248046).SetDescription("User was unbanned").GetAuthor()->SetName(name);
		co_await EditInteractionResponse(i,
			kw::embeds={ std::move(e) },
			kw::components={
				cActionRow{
					cButton{
						BUTTON_STYLE_SECONDARY,
						fmt::format("DLT#{}", i.GetUser().GetId()),
						kw::label="Dismiss"
					}
				}
			}
		);
	}
}

cTask<>
cGreekBot::process_ban_ctx_menu(cAppCmdInteraction& i, eSubcommand subcmd) {
	bool bAck = false;
	try {
		/* Check that the invoking member has appropriate permissions */
		if (!(i.GetMember()->GetPermissions() & PERM_BAN_MEMBERS))
			co_return co_await RespondToInteraction(i, kw::flags=MESSAGE_FLAG_EPHEMERAL, kw::content="You can't do that. You're missing the `BAN_MEMBERS` permission");
		/* Retrieve the user to be banned */
		auto[user, member] = i.GetOptions().front().GetValue<APP_CMD_OPT_USER>();
		/* Make sure we're not banning ourselves */
		if (user->GetId() == GetUser()->GetId())
			co_return co_await RespondToInteraction(i, kw::flags=MESSAGE_FLAG_EPHEMERAL, kw::content=get_no_ban_msg(subcmd));
		/* If the subcommand is about banning trolls specifically, ban right away */
		if (subcmd != SUBCMD_USER) {
			co_await RespondToInteraction(i);
			bAck = true;
			cMessageParams msg = co_await process_ban(i, subcmd, *user, std::chrono::days(7), {}, {});
			co_return co_await SendInteractionFollowupMessage(i, msg);
		}
		/* Otherwise, retrieve the user's display name in the guild... */
		std::string_view display_name;
		if (member && !member->GetNick().empty())
			display_name = member->GetNick();
		else if (!user->GetGlobalName().empty())
			display_name = user->GetGlobalName();
		else
			display_name = user->GetUsername();
		/* ...and then send a modal to request optional ban reason and goodbye message */
		co_return co_await RespondToInteractionWithModal(i, cModal{
			fmt::format("ban:{}", user->GetId()),
			fmt::format("Ban @{}", display_name),
			{
				cActionRow{
					cTextInput{
						TEXT_INPUT_SHORT,
						"BAN_REASON",
						"Ban reason (optional)",
						kw::required=false
					}
				},
				cActionRow{
					cTextInput{
						TEXT_INPUT_PARAGRAPH,
						"BAN_MESSAGE",
						"Custom goodbye message (optional)",
						kw::required=false
					}
				}
			}
		});
	} catch (const std::exception& e) {
		cUtils::PrintErr("process_ban_ctx_menu() {}", e.what());
	}
	/* Reply with error message */
	cMessageParams msg { kw::flags=MESSAGE_FLAG_EPHEMERAL, kw::content="An unexpected error has occurred. Try again later." };
	co_await (bAck ? SendInteractionFollowupMessage(i, msg) : RespondToInteraction(i, msg));
}
cTask<>
cGreekBot::process_ban_modal(cModalSubmitInteraction& i) {
	/* Acknowledge the interaction right away */
	co_await RespondToInteraction(i);
	try {
		/* Retrieve submitted options */
		std::string_view reason, goodbye;
		for (auto &action_row: i.GetComponents()) {
			for (auto &component: action_row.GetComponents()) {
				auto &text_input = std::get<cTextInput>(component);
				switch (cUtils::CRC32(0, text_input.GetCustomId())) {
					case 0x23240F07: // "BAN_REASON"
						reason = text_input.GetValue();
						break;
					case 0x1A1955B8: // "BAN_MESSAGE"
						goodbye = text_input.GetValue();
						break;
					default:
						throw std::runtime_error(fmt::format("Unexpected text input id: {}", text_input.GetCustomId()));
				}
			}
		}
		/* Retrieve to-be-banned user */
		cSnowflake user_id = i.GetCustomId().substr(4); // Remove the "ban:" prefix
		cUser user = co_await GetUser(user_id); // TODO: cache user info in modal custom_ids
		/* Ban user, send appropriate DM and retrieve the confirmation message parameters */
		cMessageParams msg = co_await process_ban(i, SUBCMD_USER, user, std::chrono::days(7), reason, goodbye);
		co_return co_await SendInteractionFollowupMessage(i, msg);
	} catch (const std::exception& e) {
		cUtils::PrintErr("process_ban_modal() {}", e.what());
	}
	co_await SendInteractionFollowupMessage(i, kw::flags=MESSAGE_FLAG_EPHEMERAL, kw::content="An unexpected error has occurred. Try again later.");
}