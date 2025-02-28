#include "Database.h"
#include "GreekBot.h"

cTask<>
cGreekBot::OnMessageCreate(cMessage& msg, hSnowflake guild_id, hPartialMember member) {
	/* Make sure we're in Learning Greek and that the message author is a real user */
	if (cUser& author = msg.GetAuthor(); guild_id && *guild_id == LMG_GUILD_ID && !author.IsBotUser() && !author.IsSystemUser()) {
		HANDLER_TRY {
			/* Register message for logging purposes */
			co_await process_msglog_new_message(msg);
		} HANDLER_CATCH
		HANDLER_TRY {
			/* Register message for the leaderboard */
			co_await process_leaderboard_new_message(msg, *member);
		} HANDLER_CATCH
	}
}
cTask<>
cGreekBot::OnMessageUpdate(cMessageUpdate& msg, hSnowflake guild_id, hPartialMember member) {
	if (auto pContent = msg.GetContent(); guild_id && *guild_id == LMG_GUILD_ID && pContent) HANDLER_TRY {
		/* Update the logged message content */
		co_await process_msglog_message_update(msg);
	} HANDLER_CATCH
}
cTask<>
cGreekBot::OnMessageDelete(cSnowflake& message_id, cSnowflake& channel_id, hSnowflake guild_id) {
	/* Make sure we're in Learning Greek */
	if (guild_id && *guild_id == LMG_GUILD_ID) {
		HANDLER_TRY {
			/* Delete message from the database and report to the log channel */
			co_await process_msglog_message_delete(std::span(&message_id, 1));
		} HANDLER_CATCH
		/* Delete the starboard message from the channel and the database (if found) */
		if (int64_t sb_msg_id = co_await cDatabase::SB_RemoveAll(message_id))
			co_await DeleteMessage(LMG_CHANNEL_STARBOARD, sb_msg_id);
	}
}
cTask<>
cGreekBot::OnMessageDeleteBulk(std::span<cSnowflake> ids, cSnowflake& channel_id, hSnowflake guild_id) {
	/* Make sure we're in Learning Greek */
	if (guild_id && *guild_id == LMG_GUILD_ID) {
		HANDLER_TRY {
			co_await process_msglog_message_delete(ids);
		} HANDLER_CATCH;
		/* Delete the starboard messages from the channel and the database (if found) */
		for (cSnowflake& id : ids) {
			if (int64_t sb_msg_id = co_await cDatabase::SB_RemoveAll(id)) try {
				co_await DeleteMessage(LMG_CHANNEL_STARBOARD, sb_msg_id);
			} catch (...) {}
		}
	}
}