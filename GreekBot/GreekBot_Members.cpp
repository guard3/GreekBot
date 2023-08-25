#include "GreekBot.h"
#include "Database.h"

static const cSnowflake NEW_MEMBERS_CHANNEL_ID = 1143888492422770778;

cTask<>
cGreekBot::OnGuildMemberAdd(cSnowflake& guild_id, cMember& member) {
	if (guild_id == m_lmg_id)
		co_await cDatabase::WelcomeRegisterMember(member);
}

cTask<>
cGreekBot::OnGuildMemberUpdate(cSnowflake& guild_id, cPartialMember& member) {
	/* If a member has 1 or more roles, it means they *may* be candidates for welcoming */
	// TODO: Actually check for proficiency roles since MEE6 gives old roles to members that come back, YIKES!
	if (guild_id == m_lmg_id && !member.GetRoles().empty()) {
		/* If the member doesn't have a nickname yet... */
		if (member.GetNickname().empty()) {
			/* ...and there's no registered message... */
			if (co_await cDatabase::WelcomeGetMessage(member.GetUser()) == 0) {
				/* ...send and register a message for moderators */
				cMessage msg = co_await CreateMessage(NEW_MEMBERS_CHANNEL_ID, kw::content=fmt::format("<@{}> Just got a rank!", member.GetUser().GetId()));
				co_await cDatabase::WelcomeUpdateMessage(member.GetUser(), msg);
			}
		}
		else {
			/* If the member has a nickname, edit the registered message */
			int64_t msg_id = co_await cDatabase::WelcomeGetMessage(member.GetUser());
			if (msg_id > 0) {
				co_await EditMessage(NEW_MEMBERS_CHANNEL_ID, msg_id, kw::content=fmt::format("<@{}> Just got a nickname!", member.GetUser().GetId()));
				co_await cDatabase::WelcomeEditMessage(msg_id);
			}
		}
	}
}

cTask<>
cGreekBot::OnGuildMemberRemove(cSnowflake& guild_id, cUser& user) {
	if (guild_id == m_lmg_id) {
		for (const cSnowflake& id: co_await cDatabase::WelcomeDeleteMember(user)) {
			try {
				co_await DeleteMessage(NEW_MEMBERS_CHANNEL_ID, id);
			}
			catch (...) {}
		}
	}
}

cTask<>
cGreekBot::OnMessageDelete(cSnowflake& id, cSnowflake& channel_id, hSnowflake guild_id) {
	if (guild_id) {
		if (*guild_id == m_lmg_id && channel_id == NEW_MEMBERS_CHANNEL_ID) {
			co_await cDatabase::WelcomeDeleteMessage(id);
		}
	}
}

cTask<>
cGreekBot::OnMessageDeleteBulk(std::span<cSnowflake> ids, cSnowflake& channel_id, hSnowflake guild_id) {
	if (guild_id) {
		if (*guild_id == m_lmg_id && channel_id == 1143888492422770778) {
			// TODO: Make a query to delete all messages at once
			for (auto& id: ids)
				co_await cDatabase::WelcomeDeleteMessage(id);
		}
	}
}