#include "GreekBot.h"
#include "Utils.h"
#include "CDN.h"
/* ========== Subcommand name hashes for easy switching ============================================================= */
enum : std::uint32_t {
	SUBCMD_USER  = 0x8D93D649, // "user"
	SUBCMD_TURK  = 0x504AE1C8, // "turk"
	SUBCMD_GREEK = 0xA0F01AAE  // "greek"
};
/* ========== A static message indicating missing `BAN_MEMBERS` permission ========================================== */
static const auto MISSING_PERMISSION_MESSAGE = []{
	cPartialMessage result;
	result.SetFlags(MESSAGE_FLAG_EPHEMERAL).SetContent("You can't do that. You're missing the `BAN_MEMBERS` permission.");
	return result;
}();
/* ========== A function that returns an appropriate message for when trying to ban the bot itself ================== */
static const cPartialMessage&
get_no_ban_msg(std::uint32_t e) noexcept {
	switch (e) {
		case SUBCMD_TURK: {
			static const auto result = []{
				cPartialMessage result;
				result.SetFlags(MESSAGE_FLAG_EPHEMERAL).SetContent("Excuse me, ı'm not türk! Beep Bop... 🤖");
				return result;
			}();
			return result;
		}
		case SUBCMD_GREEK: {
			static const auto result = []{
				cPartialMessage result;
				result.SetFlags(MESSAGE_FLAG_EPHEMERAL).SetContent("Όπα κάτσε, τι έκανα; Είμαι καλό μποτ εγώ. Beep Bop... 🤖");
				return result;
			}();
			return result;
		}
		default: {
			static const auto result = []{
				cPartialMessage result;
				result.SetFlags(MESSAGE_FLAG_EPHEMERAL).SetContent("I'm not gonna ban myself, I'm a good bot. Beep Bop... 🤖");
				return result;
			}();
			return result;
		}
	}
}
/* ========== Process ban command =================================================================================== */
cTask<>
cGreekBot::process_ban(cAppCmdInteraction& i) HANDLER_BEGIN {
	using namespace std::chrono;
	/* Check that the invoking user has appropriate permissions, for extra measure */
	if (!(i.GetMember()->GetPermissions() & PERM_BAN_MEMBERS))
		co_return co_await InteractionSendMessage(i, MISSING_PERMISSION_MESSAGE);
	/* Collect options */
	auto& subcommand = i.GetOptions().front();
	/* Temporary check */
	if (subcommand.GetName() == "temp") {
		co_return co_await InteractionSendMessage(i, cPartialMessage()
			.SetContent("This command isn't ready yet, soldier!")
			.SetFlags(MESSAGE_FLAG_EPHEMERAL)
		);
	}
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
				break;
			case 0xDEBA72DF: // "format" TODO: find a better name for this
			default:
				break;
		}
	}
	/* Make sure we're not banning ourselves */
	const auto sc = cUtils::CRC32(0, subcommand.GetName());
	if (i.GetUser().GetId() == GetUser().GetId())
		co_return co_await InteractionSendMessage(i, get_no_ban_msg(sc));
	/* Ban */
	co_await process_ban(i, sc, user->GetId(), user->GetAvatar(), user->GetUsername(), user->GetDiscriminator(), delete_messages, reason, goodbye);
} HANDLER_END
/* ========== Process unban button ================================================================================== */
cTask<>
cGreekBot::process_unban(cMsgCompInteraction& i, cSnowflake user_id) HANDLER_BEGIN {
	/* Make sure that the invoking user has the appropriate permissions */
	if (!(i.GetMember()->GetPermissions() & PERM_BAN_MEMBERS))
		co_return co_await InteractionSendMessage(i, MISSING_PERMISSION_MESSAGE);
	/* Unban user */
	co_await InteractionDefer(i);
	try {
		co_await RemoveGuildBan(*i.GetGuildId(), user_id);
	} catch (xDiscordError& e) {
		/* Ban not found, but that's fine */
	}
	cMessageUpdate response;
	auto& embed = response.EmplaceEmbeds(i.GetMessage().MoveEmbeds()).front();
	auto name = embed.GetAuthor()->GetName();
	name.remove_suffix(11);
	embed.ResetFields();
	embed.SetColor(0x248046).SetDescription("User was unbanned").GetAuthor()->SetName(name);
	co_await InteractionEditMessage(i, response.SetComponents({
		cActionRow{
			cButton{
				BUTTON_STYLE_SECONDARY,
				std::format("DLT#{}", i.GetUser().GetId()),
				"Dismiss"
			}
		}
	}), i.GetMessage());
} HANDLER_END
/* ========== Process ban context menu ============================================================================== */
cTask<>
cGreekBot::process_ban_ctx_menu(cAppCmdInteraction& i, std::string_view subcommand) HANDLER_BEGIN {
	/* Check that the invoking member has appropriate permissions */
	if (!(i.GetMember()->GetPermissions() & PERM_BAN_MEMBERS))
		co_return co_await InteractionSendMessage(i, MISSING_PERMISSION_MESSAGE);
	/* Retrieve the user to be banned */
	auto[user, member] = i.GetOptions().front().GetValue<APP_CMD_OPT_USER>();
	/* Make sure we're not banning ourselves */
	auto sc = cUtils::CRC32(0, subcommand);
	if (user->GetId() == GetUser().GetId())
		co_return co_await InteractionSendMessage(i, get_no_ban_msg(sc));
	/* If the subcommand is about banning trolls specifically, ban right away */
	if (sc != SUBCMD_USER)
		co_return co_await process_ban(i, sc, user->GetId(), user->GetAvatar(), user->GetUsername(), user->GetDiscriminator(), std::chrono::days(7), {}, {});
	/* Otherwise, retrieve the user's display name in the guild... */
	std::string_view display_name;
	if (member && !member->GetNick().empty())
		display_name = member->GetNick();
	else if (!user->GetGlobalName().empty())
		display_name = user->GetGlobalName();
	else
		display_name = user->GetUsername();
	/* ...and then send a modal to request optional ban reason and goodbye message */
	co_await InteractionSendModal(i, cModal{
		std::format("ban:{}:{}:{}:{}", user->GetId(), user->GetAvatar(), user->GetUsername(), user->GetDiscriminator()),
		std::format("Ban @{}", display_name),
		{
			cActionRow{
				cTextInput{
					TEXT_INPUT_SHORT,
					"BAN_REASON",
					"Ban reason (optional)",
					false
				}
			},
			cActionRow{
				cTextInput{
					TEXT_INPUT_PARAGRAPH,
					"BAN_MESSAGE",
					"Custom goodbye message (optional)",
					false
				}
			}
		}
	});
} HANDLER_END
/* ========== Process ban modal ===================================================================================== */
cTask<>
cGreekBot::process_ban_modal(cModalSubmitInteraction& i, std::string_view custom_id) HANDLER_BEGIN {
	/* Check that the invoking member has appropriate permissions */
	if (!(i.GetMember()->GetPermissions() & PERM_BAN_MEMBERS))
		co_return co_await InteractionSendMessage(i, MISSING_PERMISSION_MESSAGE);
	/* A simple exception for when the modal id (for whatever reason) is invalid */
	struct _ : std::exception {
		const char* what() const noexcept override { return "Invalid modal id"; }
	};
	/* Parse the custom id with the format "user_id:avatar:username:discriminator" */
	std::size_t p = 0, q = 0;
	if (p = custom_id.find(':', q); p == std::string_view::npos) throw _();
	cSnowflake user_id = custom_id.substr(q, p++ - q);
	if (q = custom_id.find(':', p); q == std::string_view::npos) throw _();
	auto avatar = custom_id.substr(p, q++ - p);
	if (p = custom_id.find(':', q); p == std::string_view::npos) throw _();
	auto username = custom_id.substr(q, p++ - q);
	auto disc = cUtils::ParseInt<std::uint16_t>(custom_id.substr(p));
	/* Make sure we're not banning ourselves */
	if (user_id == GetUser().GetId())
		co_return co_await InteractionSendMessage(i, get_no_ban_msg(SUBCMD_USER));
	/* Retrieve submitted options */
	std::string_view reason, goodbye;
	for (auto& action_row: i.GetComponents()) {
		for (auto& component: action_row.GetComponents()) {
			auto& text_input = std::get<cTextInput>(component);
			switch (cUtils::CRC32(0, text_input.GetCustomId())) {
				case 0x23240F07: // "BAN_REASON"
					reason = text_input.GetValue();
					break;
				case 0x1A1955B8: // "BAN_MESSAGE"
					goodbye = text_input.GetValue();
					break;
				default:
					throw std::runtime_error(std::format("Unexpected text input id: \"{}\"", text_input.GetCustomId()));
			}
		}
	}
	/* Ban */
	co_await process_ban(i, SUBCMD_USER, user_id, avatar, username, disc, std::chrono::days(7), reason, goodbye);
} HANDLER_END
/* ========== The main ban logic ==================================================================================== */
cTask<>
cGreekBot::process_ban(cInteraction& i, std::uint32_t sc, const cSnowflake& user_id, std::string_view hash, std::string_view username, std::uint16_t discr, std::chrono::seconds delete_messages, std::string_view reason, std::string_view goodbye) {
	/* Acknowledge interaction */
	co_await InteractionDefer(i, true);
	/* Update reason and goodbye message */
	switch (sc) {
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
				"Την πέθανα, την σκότωσα την πουτάνα\n    \\- Ηλίας Ψινάκης",
				"Λοιπόν, ήρθε ο Πούρσας να με κάνει νταντά ντιέμ επειδή έκανα screenshot τα χόμο μηνύματα που μου έστελνε το Γατάκι. Όλο μιμς και πλακίτσα για άλλους αλλά μόλις σκάσει το ραντ πάνω τους το χιουμοράκι πάει περίπατο και αρχίζουν να κατεβάζουν μιουτς και μπανς. Και δώσε κλάμαααα οι σόυγιακς \"μου έκανε προσωπική επίθεση σνιφ σνιφ παλιοεντζλορντττ\". Μετά τις κωλοτούμπες στα γλωσσολογικά μόλις είπα ότι δεν γουστάρω ιμπεριαλιστές νεοθωμανούς γενοκτόνους εγκληματίες έβγαλες αιμορροίδες από το μπατχερτιλίκι και άρχισες τα καρενίστικα. Ούτε καν τους τύπους δεν κρατάς πλέον, το παίζεις και Πόντιος ξεφτιλισμένη ρεβιζιονιστική κατσαριδούλα. Φαίνεται από τα logs βέβαια πως τα ίδια κάνεις με όλους, μόλις κάποιος κάνει expose πόσο crackpots είστε. Αλλά τι λέμε τώρα, εδώ έχεις πει -χωρίς να ντρέπεσαι- πως \"δεν είναι δημοκρατικός ο σέρβερ\".",
				"Ρώτα τον wannabe σανταυμαρίτη καρπαζοεισπράκτορα να σου πει τι κάνουν οι πραγματικοί αριστεροί Σφακιώτες στο Σπανοχώρι σε πρακτορίσκους σαν εσένα. Δυο αργόσχολοι παρθένοι τσατάκηδες είστε εσύ και ο άλλος ο κλόουν που παπαγαλίζετε σαν στούρνοι ότι μαλακία δείτε στην lifo. Οπότε βούλωνε και μάθε τη θέση σου. Ο καλύτερος από εσάς το πολύ να κάνει κανένα ιδιαίτερο αύριο σε οθωμανικά mantrain για να ταϊσει την πουτάνα την μάνα του. Τώρα τράβα να με μπανάρεις παιδάκι και να μου κάνεις τα τρία δύο μην τυχόν και κόψει η κυκλομαλακία σας. Τουλάχιστον πλέον ξέρεις πως μόνο σε ανυποψίαστους ξένους περνάνε οι παπαριές σας."
			};
			reason = "Greek troll";
			goodbye = goodbyes[cUtils::Random(0u, std::size(goodbyes) - 1)];
		}	break;
		default:
			if (reason.empty())
				reason = "Unspecified";
			if (goodbye.empty())
				goodbye = reason;
			break;
	}
	/* Create the embed of the confirmation message */
	cPartialMessage response;
	cEmbed& embed = response.EmplaceEmbeds().emplace_back();
	embed.EmplaceAuthor(std::format("{} was banned", username)).SetIconUrl(cCDN::GetUserAvatar(user_id, hash, discr));
	embed.SetColor(0xC43135);
	auto& fields = embed.EmplaceFields();
	fields.reserve(2);
	fields.emplace_back("Reason", reason);
	/* DM the goodbye message */
	try {
		co_await CreateDMMessage(user_id, cPartialMessage().SetContent(std::format("You've been banned from **{}** with reason:\n```{}```", m_guilds.at(*i.GetGuildId()).GetName(), goodbye)));
		/* Add the goodbye message field only after the DM was sent successfully */
		if (reason != goodbye)
			fields.emplace_back("Goodbye message", goodbye);
	} catch (...) {
		/* Couldn't send ban reason in DMs, the user may not be a member of the guild but that's fine */
	}
	/* Ban */
	co_await CreateGuildBan(*i.GetGuildId(), user_id, delete_messages, reason);
	/* Send confirmation message */
	co_await InteractionSendMessage(i, response.SetComponents({
		cActionRow{
			cButton{
				BUTTON_STYLE_DANGER,
				std::format("BAN#{}", user_id),
				"Revoke ban"
			},
			cButton{
				BUTTON_STYLE_SECONDARY,
				std::format("DLT#{}", i.GetUser().GetId()),
				"Dismiss"
			}
		}
	}));
}