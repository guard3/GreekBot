#include "Database.h"
#include "GreekBot.h"

static const cSnowflake NEW_MEMBERS_CHANNEL_ID = 1143888492422770778;

cTask<>
cGreekBot::OnGuildMemberAdd(cSnowflake& guild_id, cMember& member) {
	if (guild_id == m_lmg_id) {
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
	if (guild_id == m_lmg_id && !member.GetRoles().empty() && !member.IsPending()) {
		/* Check if there's a message registered in the database for this member */
		const int64_t msg_id = co_await cDatabase::WC_GetMessage(member);
		if (msg_id == 0 && member.GetNickname().empty()) {
			cMessage msg = co_await CreateMessage(
				NEW_MEMBERS_CHANNEL_ID,
				kw::content=fmt::format("<@{}> Just got a rank!", member.GetUser().GetId()),
				kw::components={
					cActionRow{
						cButton{
							BUTTON_STYLE_PRIMARY,
							fmt::format("NCK#{}", member.GetUser().GetId()), // Save the member id
							kw::label="Assign nickname"
						},
						cButton{
							BUTTON_STYLE_SECONDARY,
							fmt::format("DLT#{}", GetUser()->GetId()), // Save the GreekBot id as the author
							kw::label="Dismiss"
						}
					}
				}
			);
			co_await cDatabase::WC_UpdateMessage(member.GetUser(), msg);
		}
		else if (msg_id > 0 && !member.GetNickname().empty()) {
			/* If the message is unedited and the member has a nickname, edit the message
			 * If editing fails, in cases like where the original message is deleted, that's fine */
			try {
				co_await EditMessage(NEW_MEMBERS_CHANNEL_ID, msg_id,
					kw::content = fmt::format("<@{}> Just got a nickname!", member.GetUser().GetId()),
					kw::components={
						cActionRow{
							cButton{
								BUTTON_STYLE_SECONDARY,
								fmt::format("DLT#{}", GetUser()->GetId()), // Save the GreekBot id as the author
								kw::label="Dismiss"
							}
						}
					}
				);
			}
			catch (const xDiscordError&) {}
			co_await cDatabase::WC_EditMessage(msg_id);
		}
	}
}

cTask<>
cGreekBot::OnGuildMemberRemove(cSnowflake& guild_id, cUser& user) {
	if (guild_id == m_lmg_id) {
		if (uint64_t msg_id = co_await cDatabase::WC_DeleteMember(user); msg_id != 0) try {
			co_await DeleteMessage(NEW_MEMBERS_CHANNEL_ID, msg_id);
		}
		catch (...) {}
	}
}