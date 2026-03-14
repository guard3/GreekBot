#include "CDN.h"
#include "GreekBot.h"

cTask<>
cGreekBot::OnGuildMemberAdd(cSnowflake& guild_id, cMember& member) {
	if (guild_id != LMG_GUILD_ID)
		co_return;

	HANDLER_TRY {
		co_await process_nick_new_member(member);
	} HANDLER_CATCH
	HANDLER_TRY {
		co_await process_usrlog_new_member(member);
	} HANDLER_CATCH
}

cTask<>
cGreekBot::OnGuildMemberUpdate(cSnowflake& guild_id, cMemberUpdate& member) {
	if (guild_id == LMG_GUILD_ID) HANDLER_TRY {
		co_await process_nick_member_update(member);
	} HANDLER_CATCH
}

cTask<>
cGreekBot::OnGuildMemberRemove(cSnowflake& guild_id, cUser& user) {
	if (guild_id != LMG_GUILD_ID)
		co_return;

	HANDLER_TRY {
		co_await process_nick_member_remove(user);
	} HANDLER_CATCH
	HANDLER_TRY {
		co_await process_usrlog_member_remove(user);
	} HANDLER_CATCH
}