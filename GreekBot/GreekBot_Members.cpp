#include "GreekBot.h"
#include "Database.h"
#include "Utils.h"

cTask<>
cGreekBot::OnGuildMemberAdd(cSnowflake& guild_id, cMember& member) {
	if (guild_id == m_lmg_id)
		co_await cDatabase::RegisterWelcome(*member.GetUser());
}

cTask<>
cGreekBot::OnGuildMemberUpdate(cSnowflake& guild_id, cPartialMember& member) {
	/* If a member has 1 or more roles, it means they *may* be candidates for welcoming */
	if (guild_id == m_lmg_id && !member.GetRoles().empty()) {
		/* If the member doesn't have a nickname yet, send a message for moderators */
		if (member.GetNickname().empty()) {
			if (co_await cDatabase::WelcomeHasNullMessage(member.GetUser())) {
				cMessage msg = co_await CreateMessage(1143888492422770778, kw::content=fmt::format("<@{}> Just got a rank!", member.GetUser().GetId()));
				co_await cDatabase::RegisterMessage(member.GetUser(), msg);
			}
		}
		else {
			/* If the member has a nickname, edit the registered message */
			// TODO: Mark message as edited in database to avoid editing messages every time a member is updated
			cSnowflake msg_id = co_await cDatabase::WelcomeGetMessage(member.GetUser());
			if (msg_id != 0) {
				co_await EditMessage(1143888492422770778, msg_id, kw::content=fmt::format("<@{}> Just got a nickname!", member.GetUser().GetId()));
			}
		}
	}
}

cTask<>
cGreekBot::OnGuildMemberRemove(cSnowflake& guild_id, cUser& user) {
	// TODO: Delete welcome message when user leaves the server
	co_return;
}
