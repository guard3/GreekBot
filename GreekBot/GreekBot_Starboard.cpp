#include "GreekBot.h"
#include "Database.h"

static const cSnowflake HOLY_EMOJI_ID = 409075809723219969;
static const cSnowflake HOLY_CHANNEL_ID = 978993330694266920;

static constexpr int REACTION_THRESHOLD = 4; // Test value

/* This array must be sorted for binary search to work */
static const cSnowflake excluded_channels[] {
	355242373380308993, // #moderators
	366366855117668383, // #controversial
	486311040477298690, // #word-of-the-day
	595658812128624670, // #deepest-lore
	598873442288271423, // #contributors
	611889781227520003, // #travel-and-meetups
	618350374348390406, // #bulletin-board
	627084297123266561, // #moderators-voice
	650331739293745172, // #native-polls
	817078394331856907, // #private-discussions
};

cTask<>
cGreekBot::OnMessageReactionAdd(cSnowflake& user_id, cSnowflake& channel_id, cSnowflake& message_id, hSnowflake guild_id, hSnowflake message_author_id, hMember member, cEmoji& emoji) {
	/* Make sure that we're in Learning Greek and that the emoji is :Holy: */
	if (!guild_id || !emoji.GetId())
		co_return;
	if (*guild_id != m_lmg_id || *emoji.GetId() != HOLY_EMOJI_ID)
		co_return;
	/* Also make sure that we're not in an excluded channel */
	if (std::binary_search(std::begin(excluded_channels), std::end(excluded_channels), channel_id))
		co_return;
	/* Also make sure that the message author id is provided */
	uhMessage pMsg;
	if (!message_author_id) {
		pMsg = cHandle::MakeUnique<cMessage>(co_await GetChannelMessage(channel_id, message_id));
		message_author_id = &pMsg->GetAuthor().GetId();
	}
	/* Check that reactions from the author don't count */
	if (*message_author_id == user_id || *message_author_id == GetUser()->GetId()) co_return;
	/* Register received reaction in the database */
	auto[sb_msg_id, num_reactions] = co_await cDatabase::SB_RegisterReaction(message_id, *message_author_id);
	/* Process */
	co_await process_reaction(channel_id, message_id, sb_msg_id, num_reactions, pMsg.get());
}

cTask<>
cGreekBot::OnMessageReactionRemove(cSnowflake& user_id, cSnowflake& channel_id, cSnowflake& message_id, hSnowflake guild_id, cEmoji& emoji) {
	/* Make sure that we're in Learning Greek and that the emoji is :Holy: */
	if (!guild_id || !emoji.GetId()) co_return;
	if (*guild_id != m_lmg_id || *emoji.GetId() != HOLY_EMOJI_ID) co_return;
	/* Also make sure that we're not in an excluded channel */
	if (std::binary_search(std::begin(excluded_channels), std::end(excluded_channels), channel_id))
		co_return;
	/* Make sure that reactions from the author don't count */
	int64_t author_id = co_await cDatabase::SB_GetMessageAuthor(message_id);
	if (author_id == 0 || author_id == user_id || author_id == GetUser()->GetId()) co_return;
	/* Remove one reaction from the database */
	auto[sb_msg_id, num_reactions] = co_await cDatabase::SB_RemoveReaction(message_id);
	/* Process */
	co_await process_reaction(channel_id, message_id, sb_msg_id, num_reactions, nullptr);
}

cTask<>
cGreekBot::process_reaction(const cSnowflake& channel_id, const cSnowflake& message_id, int64_t sb_msg_id, int64_t num_reactions, cMessage* msg) {
	/* If the number of reactions is less than the threshold, delete the starboard message if it was posted before */
	if (num_reactions < REACTION_THRESHOLD) {
		if (sb_msg_id) {
			co_await DeleteMessage(HOLY_CHANNEL_ID, sb_msg_id);
			co_await cDatabase::SB_RemoveMessage(message_id);
		}
		co_return;
	}
	/* Otherwise, prepare the message content with the :Holy: count */
	const char* reaction;
	switch (num_reactions) {
		case REACTION_THRESHOLD:
			reaction = "<:Holy:409075809723219969>";
			break;
		case REACTION_THRESHOLD + 1:
			reaction = "<:magik:1167594533849149450>";
			break;
		default:
			reaction = "<a:spin:1167594572050866207>";
			break;
	}
	auto content = fmt::format("{} **{}** https://discord.com/channels/{}/{}/{}", reaction, num_reactions, m_lmg_id, channel_id, message_id);
	/* If there is a message id registered in the database, edit the message with the new number of reactions */
	if (sb_msg_id) {
		co_await EditMessage(HOLY_CHANNEL_ID, sb_msg_id, kw::content=std::move(content));
		co_return;
	}
	/* Make sure that we have the message object available */
	std::optional<cMessage> opt;
	if (!msg)
		msg = &opt.emplace(co_await GetChannelMessage(channel_id, message_id));
	cMember author_member = co_await GetGuildMember(m_lmg_id, msg->GetAuthor().GetId());
	cUser&  author_user = *author_member.GetUser();
	/* Prepare the message preview embed */
	std::vector<cEmbed> embed_vector;
	cEmbed& preview = embed_vector.emplace_back(
		kw::author=cEmbedAuthor{
			author_user.MoveUsername(),
			kw::icon_url=author_user.MoveAvatarUrl()
		},
		kw::color=get_lmg_member_color(author_member),
		kw::timestamp=msg->GetTimestamp()
	);
	/* First, check if the original message has any media attachments that need to be previewed */
	bool bProcessed = false;
	if (auto attachments = msg->GetAttachments(); !attachments.empty()) {
		/* Search for at least one image and one video attachment */
		cAttachment* pImage = nullptr, *pVideo = nullptr;
		for (cAttachment& a : attachments) {
			std::string_view content_type = a.GetContentType();
			if (content_type.starts_with("image/")) {
				if (pImage) continue;
				pImage = &a;
			}
			if (content_type.starts_with("video/")) {
				if (pVideo) continue;
				pVideo = &a;
			}
		}
		/* If an image is found, save it in the embedded preview */
		if (pImage) {
			preview.SetImage(pImage->MoveUrl());
			if (!pVideo)
				preview.SetDescription(msg->MoveContent());
			bProcessed = true;
		}
		/* If a video is found, add its link to the preview content since discord doesn't support client video embeds */
		if (pVideo) {
			if (msg->GetContent().empty()) {
				preview.SetTitle("Video");
				preview.SetDescription(pVideo->MoveUrl());
			} else {
				preview.SetDescription(fmt::format("{}\n\n**Video**\n{}", msg->GetContent(), pVideo->GetUrl()));
			}
			bProcessed = true;
		}
	}
	/* If no suitable attachments are found, check any embedded thumbnails of images */
	if (!bProcessed) {
		if (auto embeds = msg->GetEmbeds(); !embeds.empty()) {
			std::string url;
			for (cEmbed& e : embeds) {
				if (auto t = e.GetThumbnail()) {
					if (t->GetUrl().starts_with("https://media.tenor.com/")) {
						/* NASTY HACK! If we detect that the embedded link is a tenor gif, we modify the url to get a link to a .gif file! */
						url = t->MoveUrl();
						url[url.rfind('/') - 1] = 'C';
						url.replace(url.size() - 3, 3, "gif");
						break;
					}
					if (t->GetUrl() == msg->GetContent()) {
						/* If the message content is the link to the embedded image, save it to be embedded */
						url = t->MoveUrl();
						break;
					}
				}
			}
			/* If a suitable thumbnail is found, save it to the message preview */
			if (!url.empty()) {
				preview.SetImage(std::move(url));
				bProcessed = true;
			}
		}
	}
	/* If no embedded media was processed, save the message content as is */
	if (!bProcessed)
		preview.SetDescription(msg->MoveContent());
	/* Send the starboard message and save it in the database */
	cMessage sb_msg = co_await CreateMessage(HOLY_CHANNEL_ID,
		kw::content=std::move(content),
		kw::embeds=std::move(embed_vector)
	);
	co_await cDatabase::SB_RegisterMessage(message_id, sb_msg.GetId());
}
/* ========== Delete messages when all reactions are removed or when the original message is deleted ================ */
cTask<>
cGreekBot::OnMessageReactionRemoveAll(cSnowflake& channel_id, cSnowflake& message_id, hSnowflake guild_id) {
	/* Make sure we're in Learning Greek */
	if (!guild_id) co_return;
	if (*guild_id != m_lmg_id) co_return;
	/* Delete the message from the channel and the database (if found) */
	if (int64_t sb_msg_id = co_await cDatabase::SB_RemoveAll(message_id))
		co_await DeleteMessage(HOLY_CHANNEL_ID, sb_msg_id);
}
cTask<>
cGreekBot::OnMessageReactionRemoveEmoji(cSnowflake& channel_id, cSnowflake& message_id, hSnowflake guild_id, cEmoji& emoji) {
	/* Make sure we're in Learning Greek and that the emoji is :Holy: */
	if (!guild_id || !emoji.GetId()) co_return;
	if (*guild_id != m_lmg_id || *emoji.GetId() != HOLY_EMOJI_ID) co_return;
	/* Delete the message from the channel and the database (if found) */
	if (int64_t sb_msg_id = co_await cDatabase::SB_RemoveAll(message_id))
		co_await DeleteMessage(HOLY_CHANNEL_ID, sb_msg_id);
}
cTask<>
cGreekBot::OnMessageDelete(cSnowflake& message_id, cSnowflake& channel_id, hSnowflake guild_id) {
	/* Make sure we're in Learning Greek */
	if (!guild_id) co_return;
	if (*guild_id != m_lmg_id) co_return;
	/* Delete the starboard message from the channel and the database (if found) */
	if (int64_t sb_msg_id = co_await cDatabase::SB_RemoveAll(message_id))
		co_await DeleteMessage(HOLY_CHANNEL_ID, sb_msg_id);
}
cTask<>
cGreekBot::OnMessageDeleteBulk(std::span<cSnowflake> ids, cSnowflake& channel_id, hSnowflake guild_id) {
	/* Make sure we're in Learning Greek */
	if (!guild_id) co_return;
	if (*guild_id != m_lmg_id) co_return;
	/* Delete the starboard message from the channel and the database (if found) */
	for (cSnowflake& id : ids) {
		if (int64_t sb_msg_id = co_await cDatabase::SB_RemoveAll(id))
			co_await DeleteMessage(HOLY_CHANNEL_ID, sb_msg_id);
	}
}