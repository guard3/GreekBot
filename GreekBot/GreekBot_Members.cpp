#include "CDN.h"
#include "DBWelcoming.h"
#include "GreekBot.h"

static const cSnowflake NEW_MEMBERS_CHANNEL_ID = 1143888492422770778;

cTask<>
cGreekBot::OnGuildMemberAdd(cSnowflake& guild_id, cMember& member) {
	if (guild_id == LMG_GUILD_ID) {
		HANDLER_TRY {
			if (const uint64_t old_msg_id = co_await cWelcomingDAO(co_await cDatabase::CreateTransaction()).RegisterMember(member); old_msg_id != 0)
				co_await DeleteMessage(NEW_MEMBERS_CHANNEL_ID, old_msg_id);
		} HANDLER_CATCH
	}
}

cTask<>
cGreekBot::OnGuildMemberUpdate(cSnowflake& guild_id, cMemberUpdate& member) {
	/* If a member has 1 or more roles, it means they *may* be candidates for welcoming */
	// TODO: Actually check for proficiency roles since MEE6 gives old roles to members that come back, YIKES!
	if (guild_id == LMG_GUILD_ID && !member.GetRoles().empty() && !member.IsPending()) {
		/* Check if there's a message registered in the database for this member */
		auto txn = co_await cDatabase::CreateTransaction();
		cWelcomingDAO dao(txn);
		const int64_t msg_id = co_await dao.GetMessage(member);
		if (msg_id == 0 && member.GetNickname().empty()) {
			cMessage msg = co_await CreateMessage(NEW_MEMBERS_CHANNEL_ID, cPartialMessage()
				.SetContent(std::format("<@{}> Just got a rank!{}", member.GetUser().GetId(), member.GetFlags() & MEMBER_FLAG_DID_REJOIN ? "\n-# They rejoined the server." : ""))
				.SetComponents({
					cActionRow{
						cButton{
							BUTTON_STYLE_PRIMARY,
							std::format("NCK#{}", member.GetUser().GetId()), // Save the member id
							"Assign nickname"
						},
						cButton{
							BUTTON_STYLE_SECONDARY,
							std::format("DLT#{}", GetUser().GetId()), // Save the GreekBot id as the author
							"Dismiss"
						}
					}
				})
			);
			co_await dao.UpdateMessage(member.GetUser(), msg);
		}
		else if (msg_id > 0 && !member.GetNickname().empty()) {
			/* If the message is unedited and the member has a nickname, edit the message
			 * If editing fails, in cases like where the original message is deleted, that's fine */
			try {
				co_await EditMessage(NEW_MEMBERS_CHANNEL_ID, msg_id, cMessageUpdate()
					.SetContent(std::format("<@{}> Just got a nickname!{}", member.GetUser().GetId(), member.GetFlags() & MEMBER_FLAG_DID_REJOIN ? "\n-# They rejoined the server." : ""))
					.SetComponents({
						cActionRow{
							cButton{
								BUTTON_STYLE_SECONDARY,
								std::format("DLT#{}", GetUser().GetId()), // Save the GreekBot id as the author
								"Dismiss"
							}
						}
					})
				);
			} catch (const xDiscordError&) {}
			co_await dao.EditMessage(msg_id);
		}
	}
}

cTask<>
cGreekBot::OnGuildMemberRemove(cSnowflake& guild_id, cUser& user) {
	if (guild_id != LMG_GUILD_ID)
		co_return;
	/* Delete the welcoming message if it exists */
	if (uint64_t msg_id = co_await cWelcomingDAO(co_await cDatabase::CreateTransaction()).DeleteMember(user); msg_id != 0) try {
		co_await DeleteMessage(NEW_MEMBERS_CHANNEL_ID, msg_id);
	} catch (...) {}
	/* Notify that the user left */
	using namespace std::chrono;
	cPartialMessage msg;
	auto& embed = msg.EmplaceEmbeds().emplace_back();
	embed.EmplaceAuthor(user.GetUsername()).SetIconUrl(cCDN::GetUserAvatar(user));
	embed.SetColor(LMG_COLOR_RED);
	embed.SetTimestamp(floor<milliseconds>(system_clock::now()));
	embed.SetDescription("🚪 Left the server");
	embed.SetFields({{ "User ID", std::format("`{}`", user.GetId()) }});
	co_await CreateMessage(LMG_CHANNEL_USER_LOG, msg);
}