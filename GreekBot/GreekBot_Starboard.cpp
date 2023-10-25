#include "GreekBot.h"
#include "Database.h"
#include "Utils.h"

static const cSnowflake HOLY_EMOJI_ID = 409075809723219969;
static const cSnowflake BOTS_CHANNEL_ID = 354924532479295498;

static constexpr int REACTION_THRESHOLD = 2; // Test value

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

	std::optional<cMessage> opt;
	chSnowflake author_id = message_author_id;

	if (!author_id)
		author_id = &opt.emplace(co_await GetChannelMessage(channel_id, message_id)).GetAuthor().GetId();

	if (*author_id == user_id) co_return;

	/* Register received reaction in the database */
	// TODO: split this into 2 separate queries cuz there's some delay and some messages get sent twice
	auto[sb_msg_id, num_reactions] = co_await cDatabase::SB_RegisterReaction(message_id, *author_id);
	/* Process */
	co_await process_reaction(channel_id, message_id, sb_msg_id, num_reactions, &opt);
}

cTask<>
cGreekBot::OnMessageReactionRemove(cSnowflake& user_id, cSnowflake& channel_id, cSnowflake& message_id, hSnowflake guild_id, cEmoji& emoji) {
	/* Make sure that we're in Learning Greek and that the emoji is :Holy: */
	if (!guild_id || !emoji.GetId()) co_return;
	if (*guild_id != m_lmg_id || *emoji.GetId() != HOLY_EMOJI_ID) co_return;

	int64_t author_id = co_await cDatabase::SB_GetMessageAuthor(message_id);
	if (author_id)
		if (author_id == user_id) co_return;
	/* Remove one reaction from the database */
	auto[sb_msg_id, num_reactions] = co_await cDatabase::SB_RemoveReaction(message_id);
	/* Process */
	co_await process_reaction(channel_id, message_id, sb_msg_id, num_reactions, nullptr);
}

cTask<>
cGreekBot::process_reaction(const cSnowflake& channel_id, const cSnowflake& message_id, int64_t sb_msg_id, int64_t num_reactions, std::optional<cMessage>* msg) {
	if (num_reactions < REACTION_THRESHOLD) {
		/* If the number of reactions is less than the threshold, delete the starboard message if it was posted before */
		if (sb_msg_id) {
			co_await DeleteMessage(BOTS_CHANNEL_ID, sb_msg_id);
			co_await cDatabase::SB_RemoveMessage(message_id);
		}
	} else {
		/* Otherwise... */
		std::optional<cMessage> opt;
		if (!msg)
			msg = &opt;
		if (!msg->has_value())
			msg->emplace(co_await GetChannelMessage(channel_id, message_id));

		const char* reaction1 = "<:Holy:409075809723219969> ";
		const char* reaction2 = "";
		const char* reaction3 = "";
		if (num_reactions > 2) {
			reaction2 = reaction1;
			if (num_reactions > 3)
				reaction3 = reaction1;
		}

		cMember author = co_await GetGuildMember(m_lmg_id, (*msg)->GetAuthor().GetId());

		auto content = fmt::format("{}{}{}**{}** https://discord.com/channels/{}/{}/{}", reaction1, reaction2, reaction3, num_reactions, m_lmg_id, channel_id, message_id);
		if (sb_msg_id) {
			/* If there is a message id registered in the database, edit the message with the new number of reactions */
			co_await EditMessage(BOTS_CHANNEL_ID, sb_msg_id, kw::content=std::move(content));
		} else {
			cEmbed e{
				kw::author=cEmbedAuthor{
					author.GetUser()->GetUsername(),
					kw::icon_url=author.GetUser()->GetAvatarUrl()
				},
				kw::color=get_lmg_member_color(author),
				kw::timestamp=(*msg)->GetTimestamp()
			};
			/* When someone posts a link to an image, that link appears in the message content.
			 * BUT, discord detects it as an image and creates an embed with it.
			 * So, we check if the message content appears as a thumbnail url in any of the embeds of the message
			 * and if that's the case, we create an image embed ourselves and leave the description empty */
			auto embeds = (*msg)->GetEmbeds();
			if (!embeds.empty()) {
				auto it = std::find_if(embeds.begin(), embeds.end(), [content = (*msg)->GetContent()](const cEmbed& e) {
					auto thumbnail = e.GetThumbnail();
					if (thumbnail)
						return thumbnail->GetUrl() == content;
					return false;
				});
				if (it == embeds.end())
					e.SetDescription((*msg)->GetContent());
				else
					e.SetImage(it->GetUrl());
			} else {
				e.SetDescription((*msg)->GetContent());
			}

			/* If there's no registered message id in the database, send a new message */
			cMessage sb_msg = co_await CreateMessage(BOTS_CHANNEL_ID,
				kw::content=std::move(content),
				kw::embeds={
					std::move(e)
				}
			);
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