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
		HANDLER_TRY {
			/* Delete the starboard message from the channel and the database (if found) */
			co_await process_starboard_message_delete(std::span(&message_id, 1));
		} HANDLER_CATCH
	}
}
cTask<>
cGreekBot::OnMessageDeleteBulk(std::span<cSnowflake> ids, cSnowflake& channel_id, hSnowflake guild_id) {
	/* Make sure we're in Learning Greek */
	if (guild_id && *guild_id == LMG_GUILD_ID) {
		HANDLER_TRY {
			/* Delete message from the database and report to the log channel */
			co_await process_msglog_message_delete(ids);
		} HANDLER_CATCH
		HANDLER_TRY {
			/* Delete the starboard messages from the channel and the database (if found) */
			co_await process_starboard_message_delete(ids);
		} HANDLER_CATCH
	}
}
cTask<>
cGreekBot::OnMessageReactionAdd(cSnowflake& user_id, cSnowflake& channel_id, cSnowflake& message_id, hSnowflake guild_id, hSnowflake message_author_id, hMember member, cEmoji& emoji) {
	/* Make sure that we're in Learning Greek */
	if (guild_id && *guild_id == LMG_GUILD_ID) {
		HANDLER_TRY {
			co_await process_starboard_reaction_add(user_id, channel_id, message_id, message_author_id, emoji);
		} HANDLER_CATCH
		HANDLER_TRY {
			co_await process_native_polls_reaction_add(user_id, channel_id, message_id, member, emoji);
		} HANDLER_CATCH
	}
}
cTask<>
cGreekBot::OnMessageReactionRemove(cSnowflake& user_id, cSnowflake& channel_id, cSnowflake& message_id, hSnowflake guild_id, cEmoji& emoji) {
	/* Make sure that we're in Learning Greek */
	if (guild_id && *guild_id == LMG_GUILD_ID) HANDLER_TRY {
		co_await process_starboard_reaction_remove(user_id, channel_id, message_id, emoji);
	} HANDLER_CATCH
}
cTask<>
cGreekBot::OnMessageReactionRemoveAll(cSnowflake& channel_id, cSnowflake& message_id, hSnowflake guild_id) {
	/* Make sure we're in Learning Greek */
	if (guild_id && *guild_id == LMG_GUILD_ID) HANDLER_TRY {
		/* Delete the message from the channel and the database (if found) */
		co_await process_starboard_message_delete(std::span(&message_id, 1));
	} HANDLER_CATCH
}
cTask<>
cGreekBot::OnMessageReactionRemoveEmoji(cSnowflake& channel_id, cSnowflake& message_id, hSnowflake guild_id, cEmoji& emoji) {
	/* Make sure we're in Learning Greek and that the emoji is :Holy: */
	if (guild_id && *guild_id == LMG_GUILD_ID && emoji == LMG_EMOJI_HOLY) HANDLER_TRY {
		/* Delete the message from the channel and the database (if found) */
		co_await process_starboard_message_delete(std::span(&message_id, 1));
	} HANDLER_CATCH
}