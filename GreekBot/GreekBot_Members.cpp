#include "CDN.h"
#include "GreekBot.h"

cTask<>
cGreekBot::OnGuildMemberAdd(cSnowflake& guild_id, cMember& member) {
	if (guild_id == LMG_GUILD_ID) HANDLER_TRY {
		co_await process_nick_new_member(member);
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