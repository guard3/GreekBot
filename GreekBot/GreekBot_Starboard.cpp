#include "GreekBot.h"
#include "Database.h"
#include "Utils.h"

static const cSnowflake HOLY_EMOJI_ID = 409075809723219969;
static const cSnowflake BOTS_CHANNEL_ID = 354924532479295498;

static constexpr int REACTION_THRESHOLD = 3; // Test value

/*
 * TODO:
 * - Check that reactions from message authors don't count
 * - Edit database table to include author_id for ^ this reason
 * - Uhhh, actually copy the message content and (oh boy) attachments, ugh...
 */

cTask<>
cGreekBot::OnMessageReactionAdd(cSnowflake& user_id, cSnowflake& channel_id, cSnowflake& message_id, hSnowflake guild_id, hSnowflake message_author_id, hMember member, cEmoji& emoji) {
	/* Make sure that we're in Learning Greek and that the emoji is :Holy: */
	if (!guild_id || !emoji.GetId()) co_return;
	if (*guild_id != m_lmg_id || *emoji.GetId() != HOLY_EMOJI_ID) co_return;
	/* Register received reaction in the database */
	auto[sb_msg_id, num_reactions] = co_await cDatabase::SB_RegisterReaction(message_id);
	/* Process */
	co_await process_reaction(channel_id, message_id, sb_msg_id, num_reactions);
}

cTask<>
cGreekBot::OnMessageReactionRemove(cSnowflake& user_id, cSnowflake& channel_id, cSnowflake& message_id, hSnowflake guild_id, cEmoji& emoji) {
	/* Make sure that we're in Learning Greek and that the emoji is :Holy: */
	if (!guild_id || !emoji.GetId()) co_return;
	if (*guild_id != m_lmg_id || *emoji.GetId() != HOLY_EMOJI_ID) co_return;
	/* Remove one reaction from the database */
	auto[sb_msg_id, num_reactions] = co_await cDatabase::SB_RemoveReaction(message_id);
	/* Process */
	co_await process_reaction(channel_id, message_id, sb_msg_id, num_reactions);
}

cTask<>
cGreekBot::process_reaction(const cSnowflake& channel_id, const cSnowflake& message_id, int64_t sb_msg_id, int64_t num_reactions) {
	if (num_reactions < REACTION_THRESHOLD) {
		/* If the number of reactions is less than the threshold, delete the starboard message if it was posted before */
		if (sb_msg_id) {
			co_await DeleteMessage(BOTS_CHANNEL_ID, sb_msg_id);
			co_await cDatabase::SB_RemoveMessage(message_id);
		}
	} else {
		/* Otherwise... */
		auto content = fmt::format("**{}** <:Holy:409075809723219969> https://discord.com/channels/{}/{}/{}", num_reactions, m_lmg_id, channel_id, message_id);
		if (sb_msg_id) {
			/* If there is a message id registered in the database, edit the message with the new number of reactions */
			co_await EditMessage(BOTS_CHANNEL_ID, sb_msg_id, kw::content=std::move(content));
		} else {
			/* If there's no registered message id in the database, send a new message */
			cMessage sb_msg = co_await CreateMessage(BOTS_CHANNEL_ID, kw::content=std::move(content));
			co_await cDatabase::SB_RegisterMessage(message_id, sb_msg.GetId());
		}
	}
	cUtils::PrintLog("Meow: {}, num_reactions: {}", sb_msg_id, num_reactions);
}
/* ========== Delete messages when all reactions are removed or when the original message is deleted ================ */
cTask<>
cGreekBot::OnMessageReactionRemoveAll(cSnowflake& channel_id, cSnowflake& message_id, hSnowflake guild_id) {
	/* Make sure we're in Learning Greek */
	if (!guild_id) co_return;
	if (*guild_id != m_lmg_id) co_return;
	/* Delete the message from the channel and the database (if found) */
	if (int64_t sb_msg_id = co_await cDatabase::SB_RemoveAll(message_id))
		co_await DeleteMessage(BOTS_CHANNEL_ID, sb_msg_id);
	cUtils::PrintLog("Deleted everything...");
}
cTask<>
cGreekBot::OnMessageReactionRemoveEmoji(cSnowflake& channel_id, cSnowflake& message_id, hSnowflake guild_id, cEmoji& emoji) {
	/* Make sure we're in Learning Greek and that the emoji is :Holy: */
	if (!guild_id || !emoji.GetId()) co_return;
	if (*guild_id != m_lmg_id || *emoji.GetId() != HOLY_EMOJI_ID) co_return;
	/* Delete the message from the channel and the database (if found) */
	if (int64_t sb_msg_id = co_await cDatabase::SB_RemoveAll(message_id))
		co_await DeleteMessage(BOTS_CHANNEL_ID, sb_msg_id);
	cUtils::PrintLog("Deleted Holies...");
}
cTask<>
cGreekBot::OnMessageDelete(cSnowflake& message_id, cSnowflake& channel_id, hSnowflake guild_id) {
	/* Make sure we're in Learning Greek */
	if (!guild_id) co_return;
	if (*guild_id != m_lmg_id) co_return;
	/* Delete the starboard message from the channel and the database (if found) */
	if (int64_t sb_msg_id = co_await cDatabase::SB_RemoveAll(message_id))
		co_await DeleteMessage(BOTS_CHANNEL_ID, sb_msg_id);
	cUtils::PrintLog("Deleted message...");
}