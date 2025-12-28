#include "CDN.h"
#include "DBTempBans.h"
#include "GreekBot.h"
#include "Utils.h"
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
/* ========== A simple parser for either absolute YYYY-MM-DD dates or XXyXXmXXwXXd durations ======================== */
static std::optional<std::chrono::sys_days>
parse_ban_duration(std::string_view fmt) {
	using namespace std::chrono;

	const char* const fmt_begin = fmt.data();
	const char* const fmt_end = fmt_begin + fmt.size();
	/* We expect the format string to start with an int */
	unsigned val = 0;
	if (auto[ptr, ec] = std::from_chars(fmt_begin, fmt_end, val); ec == std::errc{} && ptr != fmt_end) {
		if (*ptr == '-' && ptr - fmt_begin == 4) {
			/* If the result pointer points to a '-', then interpret the format string as a YYYY-MM-DD date */
			const unsigned y = val;
			auto res = std::from_chars(++ptr, fmt_end, val);
			if (res.ec == std::errc{} && res.ptr - ptr == 2 && res.ptr != fmt_end && *res.ptr == '-') {
				const unsigned m = val;
				res = std::from_chars(ptr = res.ptr + 1, fmt_end, val);
				if (res.ec == std::errc{} && fmt_end - ptr == 2 && res.ptr == fmt_end) {
					if (year_month_day ymd{ year(y), month(m), day(val) }; ymd.ok())
						return ymd;
				}
			}
		} else {
			/* In this case, the format string is a duration */
			for (unsigned y = 0, m = 0, w = 0, d = 0; val != 0;) {
				if (*ptr == 'y' && y == 0)
					y = val;
				else if (*ptr == 'm' && m == 0)
					m = val;
				else if (*ptr == 'w' && w == 0)
					w = val;
				else if (*ptr == 'd' && d == 0)
					d = val;
				else
					break;
				/* If the entire string is consumed, return the final time point */
				if (++ptr == fmt_end)
					return ceil<days>(system_clock::now() + years(y) + months(m) + weeks(w) + days(d));
				/* If not, read some more */
				auto[p, e] = std::from_chars(ptr, fmt_end, val);
				/* If any error occurred or the entire string was consumed, fail */
				if (e != std::errc{} || p == fmt_end)
					break;
				ptr = p;
			}
		}
	}
	/* If all fails, return an empty optional */
	return {};
}
/* ========== Process ban command =================================================================================== */
cTask<>
cGreekBot::process_ban(cAppCmdInteraction& i) HANDLER_BEGIN {
	using namespace std::chrono;
	/* Check that the invoking user has appropriate permissions, for extra measure */
	if (i.GetMember()->GetPermissions().TestNone(PERM_BAN_MEMBERS))
		co_return co_await InteractionSendMessage(i, MISSING_PERMISSION_MESSAGE);
	/* Collect options */
	auto& subcommand = i.GetOptions().front();
	hUser user;
	seconds delete_messages = days(7);
	std::string_view reason, goodbye, expiry_fmt;
	for (auto& opt: subcommand.GetOptions()) {
		switch (auto crc = cUtils::CRC32(0, opt.GetName())) {
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
				expiry_fmt = opt.GetValue<APP_CMD_OPT_STRING>();
				break;
			[[unlikely]]
			default:
				throw std::runtime_error(std::format("Unknown ban command option: \"{}\" 0x{:08X}", opt.GetName(), crc));
		}
	}
	/* Make sure we're not banning ourselves */
	const auto sc = cUtils::CRC32(0, subcommand.GetName());
	if (i.GetUser().GetId() == GetUser().GetId())
		co_return co_await InteractionSendMessage(i, get_no_ban_msg(sc));
	/* Ban */
	co_await process_ban(i, sc, user->GetId(), user->GetAvatar(), user->GetUsername(), user->GetDiscriminator(), delete_messages, reason, goodbye, expiry_fmt);
} HANDLER_END
/* ========== Process unban ========================================================================================= */
cTask<>
cGreekBot::process_unban(cInteraction& i, cSnowflake user_id) HANDLER_BEGIN {
	/* Make sure that the invoking user has the appropriate permissions */
	if (i.GetMember()->GetPermissions().TestNone(PERM_BAN_MEMBERS)) [[unlikely]]
		co_return co_await InteractionSendMessage(i, MISSING_PERMISSION_MESSAGE);
	/* Acknowledge interaction */
	co_await InteractionDefer(i);
	/* Create the response and update user id if necessary */
	auto msg = i.Visit(cVisitor{
		[&user_id](cAppCmdInteraction& i) {
			/* Retrieve the user to be unbanned */
			auto options = i.GetOptions();
			if (options.size() != 1) [[unlikely]]
						throw std::runtime_error("meow");
			auto[user, _] = options.front().GetValue<APP_CMD_OPT_USER>();
			/* Create the response */
			cMessageUpdate msg;
			auto& embed = msg.EmplaceEmbeds().emplace_back();
			embed.SetColor(LMG_COLOR_GREEN);
			embed.EmplaceAuthor(std::format("{} was unbanned", user->GetUsername())).SetIconUrl(cCDN::GetUserAvatar(*user));
			/* Also update the user id */
			user_id = user->GetId();
			return msg;
		},
		[](cMsgCompInteraction& i) {
			cMessageUpdate msg;
			auto& embed = msg.EmplaceEmbeds(i.GetMessage().MoveEmbeds()).at(0);
			embed.SetColor(LMG_COLOR_GREEN);
			embed.ResetFields();
			embed.GetAuthor()->SetName(std::format("{}unbanned", [name = embed.GetAuthor()->GetName()] {
				auto n = name.rfind("banned");
				return n == std::string_view::npos ? "User was " : name.substr(0, n);
			}()));
			return msg;
		},
		[](cModalSubmitInteraction&) -> cMessageUpdate {
			[[unlikely]]
			throw std::runtime_error("");
		}
	});
	/* Remove the ban from the database */
	auto txn = co_await cTransaction::New();
	co_await txn.Begin();
	co_await cTempBanDAO(txn).Remove(user_id);
	/* Unban the user */
	try {
		co_await RemoveGuildBan(*i.GetGuildId(), user_id, std::format("{} used the /unban command", i.GetUser().GetUsername()));
	} catch (xDiscordError&) {
		/* Ban not found, but that's fine */
	}
	co_await txn.Commit();
	txn.Close();
	/* Send confirmation */
	co_await InteractionEditMessage(i, msg.SetComponents({
		cActionRow{
			cButton{
				BUTTON_STYLE_SECONDARY,
				std::format("DLT#{}", i.GetUser().GetId()),
				"Dismiss"
			}
		}
	}));
	co_return;
} HANDLER_END
/* ========== Process ban context menu ============================================================================== */
cTask<>
cGreekBot::process_ban_ctx_menu(cAppCmdInteraction& i, std::string_view subcommand) HANDLER_BEGIN {
	/* Check that the invoking member has appropriate permissions */
	if (i.GetMember()->GetPermissions().TestNone(PERM_BAN_MEMBERS))
		co_return co_await InteractionSendMessage(i, MISSING_PERMISSION_MESSAGE);
	/* Retrieve the user to be banned */
	auto[user, member] = i.GetOptions().front().GetValue<APP_CMD_OPT_USER>();
	/* Make sure we're not banning ourselves */
	const auto sc = cUtils::CRC32(0, subcommand);
	if (user->GetId() == GetUser().GetId())
		co_return co_await InteractionSendMessage(i, get_no_ban_msg(sc));
	/* If the subcommand is about banning trolls specifically, ban right away */
	if (sc == SUBCMD_GREEK || sc == SUBCMD_TURK)
		co_return co_await process_ban(i, sc, user->GetId(), user->GetAvatar(), user->GetUsername(), user->GetDiscriminator(), std::chrono::days(7), {}, {}, {});
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
			},
			cActionRow{
				cTextInput{
					TEXT_INPUT_SHORT,
					"BAN_UNTIL",
					"For how long or until when to ban (optional)",
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
	if (i.GetMember()->GetPermissions().TestNone(PERM_BAN_MEMBERS))
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
	std::string_view reason, goodbye, expiry_fmt;
	for (auto& action_row: i.GetComponents()) {
		for (auto& component: action_row.GetComponents()) {
			auto& text_input = std::get<cTextInput>(component);
			switch (auto crc = cUtils::CRC32(0, text_input.GetCustomId())) {
				case 0x23240F07: // "BAN_REASON"
					reason = text_input.GetValue();
					break;
				case 0x1A1955B8: // "BAN_MESSAGE"
					goodbye = text_input.GetValue();
					break;
				case 0xA4EA5974: // "BAN_UNTIL"
					expiry_fmt = text_input.GetValue();
					break;
				[[unlikely]]
				default:
					throw std::runtime_error(std::format("Unexpected text input id: \"{}\" 0x{:08X}", text_input.GetCustomId(), crc));
			}
		}
	}
	/* Ban */
	co_await process_ban(i, SUBCMD_USER, user_id, avatar, username, disc, std::chrono::days(7), reason, goodbye, expiry_fmt);
} HANDLER_END
/* ========== The main ban logic ==================================================================================== */
cTask<>
cGreekBot::process_ban(cInteraction& i, std::uint32_t sc, const cSnowflake& user_id, std::string_view hash, std::string_view username, std::uint16_t discr, std::chrono::seconds delete_messages, std::string_view reason, std::string_view goodbye, std::string_view expiry_format) {
	using namespace std::chrono;
	/* First, check the expiry format string, since if it's invalid we need to report error with an ephemeral message */
	sys_days expiry;
	if (!expiry_format.empty()) {
		auto now = system_clock::now();
		if (auto tp = parse_ban_duration(expiry_format); tp && *tp > now) {
			/* If the format string is parsed successfully and the resulting time point is further into the future, save the parameter */
			expiry = *tp;
		} else {
			/* On error, report a help message, use an example date that's 8 months from now */
			auto t = floor<seconds>(now + months(8));
			co_return co_await InteractionSendMessage(i, cPartialMessage()
				.SetFlags(MESSAGE_FLAG_EPHEMERAL)
				.SetContent(std::format("Invalid format string!\n"
				                        "To specify a ban until a specific *future* date, specify a `YYYY-MM-DD` date.\n"
				                        "For example:\n"
				                        "- `{:%F}`, for the ban to last until <t:{:%Q}:D>\n"
				                        "To specify a ban as a duration, use any (non-zero) value combination of `y` years, `m` months, `w` weeks or `d` days.\n"
				                        "For example:\n"
				                        "- ` 25d`, for the ban to last for **25** days\n"
				                        "- `3m2d`, for the ban to last for **3** months and **2** days\n"
				                        "- `1y5w`, for the ban to last for **1** year and **5** weeks", t, t.time_since_epoch()))
			);
		}
	}
	/* Now, we can acknowledge the interaction */
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
	/* Format the expiry time point into a string */
	const auto expiry_str = expiry == sys_days{} ? std::string() : std::format("<t:{:%Q}:D>", floor<seconds>(expiry).time_since_epoch());
	/* Create the embed of the confirmation message */
	cPartialMessage response;
	cEmbed& embed = response.EmplaceEmbeds().emplace_back();
	embed.EmplaceAuthor(std::format("{} was banned{}", username, expiry_str.empty() ? "" : " temporarily")).SetIconUrl(cCDN::GetUserAvatar(user_id, hash, discr));
	embed.SetColor(LMG_COLOR_RED);
	auto& fields = embed.EmplaceFields();
	fields.reserve(3);
	if (!expiry_str.empty())
		fields.emplace_back("Until", expiry_str);
	fields.emplace_back("Reason", reason);
	/* DM the goodbye message */
	try {
		co_await CreateDMMessage(user_id, cPartialMessage().SetContent(std::format("You've been banned from **{}**{}{} with reason:\n```{}```", m_guilds.at(*i.GetGuildId()).GetName(), expiry_str.empty() ? "" : " until ", expiry_str, goodbye)));
		/* Add the goodbye message field only after the DM was sent successfully */
		if (reason != goodbye)
			fields.emplace_back("Goodbye message", goodbye);
	} catch (...) {
		/* Couldn't send ban reason in DMs, the user may not be a member of the guild but that's fine */
	}
	/* If the ban is temporary, register it to the database or delete it if it isn't temporary */
	auto txn = co_await cTransaction::New();
	co_await txn.Begin();
	co_await cTempBanDAO(txn).Register(user_id, expiry);
	/* Ban */
	co_await CreateGuildBan(*i.GetGuildId(), user_id, delete_messages, reason);
	co_await txn.Commit();
	txn.Close();

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