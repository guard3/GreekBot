#include "Database.h"
#include "GreekBot.h"

static const cSnowflake NEW_MEMBERS_CHANNEL_ID = 1143888492422770778;

cTask<>
cGreekBot::OnGuildMemberAdd(cSnowflake& guild_id, cMember& member) {
	if (guild_id == LMG_GUILD_ID) {
		if (const uint64_t old_msg_id = co_await cDatabase::WC_RegisterMember(member); old_msg_id != 0) try {
			co_await DeleteMessage(NEW_MEMBERS_CHANNEL_ID, old_msg_id);
		}
		catch (...) {}
	}
}

cTask<>
cGreekBot::OnGuildMemberUpdate(cSnowflake& guild_id, cMemberUpdate& member) {
	/* If a member has 1 or more roles, it means they *may* be candidates for welcoming */
	// TODO: Actually check for proficiency roles since MEE6 gives old roles to members that come back, YIKES!
	if (guild_id == LMG_GUILD_ID && !member.GetRoles().empty() && !member.IsPending()) {
		/* Check if there's a message registered in the database for this member */
		const int64_t msg_id = co_await cDatabase::WC_GetMessage(member);
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
			co_await cDatabase::WC_UpdateMessage(member.GetUser(), msg);
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
			co_await cDatabase::WC_EditMessage(msg_id);
		}
	}
}

cTask<>
cGreekBot::OnGuildMemberRemove(cSnowflake& guild_id, cUser& user) {
	if (guild_id == LMG_GUILD_ID) {
		if (uint64_t msg_id = co_await cDatabase::WC_DeleteMember(user); msg_id != 0) try {
			co_await DeleteMessage(NEW_MEMBERS_CHANNEL_ID, msg_id);
		} catch (...) {}
	}
}