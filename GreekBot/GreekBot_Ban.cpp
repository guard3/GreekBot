#include "GreekBot.h"

cTask<>
cGreekBot::OnInteraction_ban(const cInteraction& i) {
	/* Acknowledge the interaction first */
	co_await RespondToInteraction(i);
	try {
		/* Collect interaction data */
		chSnowflake guild_id = i.GetGuildId();
		chMember    member   = i.GetMember();
		/* Check that the invoking user has appropriate permissions, for extra measure */
		if (!(member->GetPermissions() & PERM_BAN_MEMBERS))
			co_return co_await EditInteractionResponse(i, kw::content="You can't do that. You're missing the `BAN_MEMBERS` permission.");
		/* Get the subcommand and its options */
		auto& subcommand_option = i.GetData<INTERACTION_APPLICATION_COMMAND>().Options.front();
		auto& options = subcommand_option.GetOptions();
		bool  bTurk = subcommand_option.GetName() == "turk";
		/* Collect options */
		chUser user;
		chrono::seconds delete_messages = chrono::days(7);
		const char *reason = "Unspecified";
		for (auto& opt: options) {
			if (opt.GetName() == "user")
				user = &opt.GetValue<APP_CMD_OPT_USER>();
			else if (opt.GetName() == "delete") {
				/* Convert the received value to int */
				int value = cUtils::ParseInt(opt.GetValue<APP_CMD_OPT_STRING>());
				/* Deduce delete messages duration */
				switch (value) {
					case 0:
						delete_messages = 1h;
						break;
					case 1:
						delete_messages = chrono::days(1);
						break;
					default:
						break;
				}
			}
			else if (opt.GetName() == "reason") {
				reason = opt.GetValue<APP_CMD_OPT_STRING>().c_str();
			}
		}
		/* Make sure we're not banning ourselves */
		if (user->GetId() == GetUser()->GetId())
			co_return co_await EditInteractionResponse(
				i,
				kw::content=bTurk ? "Excuse me, ı'm not türk! Beep bop... 🤖" : "I'm not gonna ban myself, I'm a good bot. Beep bop... 🤖"
			);
		/* Update reason and goodbye message */
		const char* msg;
		if (bTurk) {
			reason = "Turk troll";
			msg = "senin ananı hacı bekir efendinin şalvarına dolar yan yatırır sokağındaki caminin minaresinde öyle bir sikerim ki ezan okundu sanırsın sonra minareden indirir koca yarrağımla köyünü yağmalar herkesi ortadox yaparım seni veled-i zina seni dljs;fjaiaejadklsjkfjdsjfklsdjflkjds;afdkslfdksfdlsfs";
		}
		else msg = reason;
		/* Create the embed of the confirmation message */
		cEmbed e {
			kw::author={
				cUtils::Format("%s#%s was banned", user->GetUsername(), user->GetDiscriminator()),
				kw::icon_url=user->GetAvatarUrl()
			},
			kw::fields = {{"Reason", reason}}
		};
		/* DM the goodbye message */
		try {
			co_await CreateDMMessage(
				user->GetId(),
				kw::content=cUtils::Format("You've been banned from **%s** with reason:\n```%s```", m_guilds.at(*guild_id)->GetName(), msg)
			);
			if (bTurk)
				e.AddField("Goodbye message", msg);
		}
		catch (...) {
			/* Couldn't send ban reason in DMs, the user may not be a member of the guild but that's fine */
		}
		/* Ban */
		co_await CreateGuildBan(*guild_id, user->GetId(), delete_messages, reason);
		/* Send confirmation message */
		co_return co_await EditInteractionResponse(
			i,
			kw::components = {
				cActionRow{
					cButton<BUTTON_STYLE_DANGER>{
						cUtils::Format("BAN#%s", user->GetId().ToString()),
						kw::label = "Revoke ban"
					},
					cButton<BUTTON_STYLE_SECONDARY>{
						cUtils::Format("DLT#%s", member->GetUser()->GetId().ToString()),
						kw::label = "Dismiss"
					}
				}
			},
			kw::embeds = {std::move(e)}
		);
	}
	catch (...) {}
	co_await EditInteractionResponse(i, kw::content="An unexpected error has occurred, try again later.");
}

cTask<>
cGreekBot::OnInteraction_unban(const cInteraction& i, const cSnowflake& user_id) {
	if (chMember member = i.GetMember()) {
		/* Make sure that the invoking user has the appropriate permissions */
		if (!(member->GetPermissions() & PERM_BAN_MEMBERS)) {
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
			auto e = i.GetMessage()->Embeds[0];
			auto& s = e.GetAuthor()->GetName();
			s.erase(s.end()-11, s.end());
			e.SetFields(nullptr).SetDescription("User was unbanned");
			co_await EditInteractionResponse(i, kw::components=nil, kw::embeds={std::move(e)});
		}
	}
}