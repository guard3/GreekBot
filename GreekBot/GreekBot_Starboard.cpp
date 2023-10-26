#include "GreekBot.h"
#include "Database.h"

static const cSnowflake HOLY_EMOJI_ID = 409075809723219969;
static const cSnowflake BOTS_CHANNEL_ID = 354924532479295498;

static constexpr int REACTION_THRESHOLD = 2; // Test value

cTask<>
cGreekBot::OnMessageReactionAdd(cSnowflake& user_id, cSnowflake& channel_id, cSnowflake& message_id, hSnowflake guild_id, hSnowflake message_author_id, hMember member, cEmoji& emoji) {
	/* Make sure that we're in Learning Greek and that the emoji is :Holy: */
	if (!guild_id || !emoji.GetId()) co_return;
	if (*guild_id != m_lmg_id || *emoji.GetId() != HOLY_EMOJI_ID) co_return;
	/* Also make sure that the message author id is provided */
	uhMessage pMsg;
	if (!message_author_id) {
		pMsg = cHandle::MakeUnique<cMessage>(co_await GetChannelMessage(channel_id, message_id));
		message_author_id = &pMsg->GetAuthor().GetId();
	}
	/* Check that reactions from the author don't count */
	if (*message_author_id == user_id || *message_author_id == GetUser()->GetId()) co_return;
	/* Register received reaction in the database */
	// TODO: split this into 2 separate queries cuz there's some delay and some messages get sent twice
	auto[sb_msg_id, num_reactions] = co_await cDatabase::SB_RegisterReaction(message_id, *message_author_id);
	/* Process */
	co_await process_reaction(channel_id, message_id, sb_msg_id, num_reactions, pMsg.get());
}

cTask<>
cGreekBot::OnMessageReactionRemove(cSnowflake& user_id, cSnowflake& channel_id, cSnowflake& message_id, hSnowflake guild_id, cEmoji& emoji) {
	/* Make sure that we're in Learning Greek and that the emoji is :Holy: */
	if (!guild_id || !emoji.GetId()) co_return;
	if (*guild_id != m_lmg_id || *emoji.GetId() != HOLY_EMOJI_ID) co_return;
	/* Make sure that reactions from the author don't count */
	int64_t author_id = co_await cDatabase::SB_GetMessageAuthor(message_id);
	if (author_id)
		if (author_id == user_id || author_id == GetUser()->GetId()) co_return;
	/* Remove one reaction from the database */
	auto[sb_msg_id, num_reactions] = co_await cDatabase::SB_RemoveReaction(message_id);
	/* Process */
	co_await process_reaction(channel_id, message_id, sb_msg_id, num_reactions, nullptr);
}

cTask<>
cGreekBot::process_reaction(const cSnowflake& channel_id, const cSnowflake& message_id, int64_t sb_msg_id, int64_t num_reactions, cMessage* msg) {
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
			msg = &opt.emplace(co_await GetChannelMessage(channel_id, message_id));

		const char* reaction1 = "<:Holy:409075809723219969> ";
		const char* reaction2 = "";
		const char* reaction3 = "";
		if (num_reactions > 2) {
			reaction2 = reaction1;
			if (num_reactions > 3)
				reaction3 = reaction1;
		}

		cMember author = co_await GetGuildMember(m_lmg_id, msg->GetAuthor().GetId());

		auto content = fmt::format("{}{}{}**{}** https://discord.com/channels/{}/{}/{}", reaction1, reaction2, reaction3, num_reactions, m_lmg_id, channel_id, message_id);
		if (sb_msg_id) {
			/* If there is a message id registered in the database, edit the message with the new number of reactions */
			co_await EditMessage(BOTS_CHANNEL_ID, sb_msg_id, kw::content=std::move(content));
			co_return;
		}
		/* Prepare the message preview embed */
		std::vector<cEmbed> embed_vector {
			cEmbed{
				kw::author=cEmbedAuthor{
					author.GetUser()->GetUsername(),
					kw::icon_url=author.GetUser()->GetAvatarUrl()
				},
				kw::color=get_lmg_member_color(author),
				kw::timestamp=msg->GetTimestamp()
			}
		};
		cEmbed& e = embed_vector.front();
		/* First, check if the original message has any media attachments that need to be previewed */
		bool bProcessed = false;
		if (auto attachments = msg->GetAttachments(); !attachments.empty()) {
			/* Search for at least one image and one video attachment */
			cAttachment* pImage = nullptr, *pVideo = nullptr;
			for (cAttachment& a : attachments) {
				std::string_view content_type = a.GetContentType();
				if (content_type.starts_with("image/")) {
					if (pImage)
						continue;
					pImage = &a;
				}
				if (content_type.starts_with("video/")) {
					if (pVideo)
						continue;
					pVideo = &a;
				}
			}
			/* If an image is found, save it in the embedded preview */
			if (pImage) {
				e.SetImage(pImage->MoveUrl());
				if (!pVideo)
					e.SetDescription(msg->GetContent());
				bProcessed = true;
			}
			/* If a video is found, add its link to the preview content since discord doesn't support client video embeds */
			if (pVideo) {
				if (msg->GetContent().empty()) {
					e.SetTitle("Video");
					e.SetDescription(pVideo->MoveUrl());
				} else {
					e.SetDescription(fmt::format("{}\n\n**Video**\n{}", msg->GetContent(), pVideo->GetUrl()));
				}
				bProcessed = true;
			}
		}
		/* If no suitable attachments are found, check any embedded thumbnails of images */
		if (!bProcessed) {
			if (auto embeds = msg->GetEmbeds(); !embeds.empty()) {
				std::string url;
				for (cEmbed& embed : embeds) {
					if (auto t = embed.GetThumbnail()) {
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
					e.SetImage(std::move(url));
					bProcessed = true;
				}
			}
		}
		/* If no embedded media was processed, save the message content as is */
		if (!bProcessed)
			e.SetDescription(msg->GetContent());
		/* If there's no registered message id in the database, send a new message */
		cMessage sb_msg = co_await CreateMessage(BOTS_CHANNEL_ID,
			kw::content=std::move(content),
			kw::embeds=std::move(embed_vector)
		);
		co_await cDatabase::SB_RegisterMessage(message_id, sb_msg.GetId());
	}
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
}
cTask<>
cGreekBot::OnMessageReactionRemoveEmoji(cSnowflake& channel_id, cSnowflake& message_id, hSnowflake guild_id, cEmoji& emoji) {
	/* Make sure we're in Learning Greek and that the emoji is :Holy: */
	if (!guild_id || !emoji.GetId()) co_return;
	if (*guild_id != m_lmg_id || *emoji.GetId() != HOLY_EMOJI_ID) co_return;
	/* Delete the message from the channel and the database (if found) */
	if (int64_t sb_msg_id = co_await cDatabase::SB_RemoveAll(message_id))
		co_await DeleteMessage(BOTS_CHANNEL_ID, sb_msg_id);
}
cTask<>
cGreekBot::OnMessageDelete(cSnowflake& message_id, cSnowflake& channel_id, hSnowflake guild_id) {
	/* Make sure we're in Learning Greek */
	if (!guild_id) co_return;
	if (*guild_id != m_lmg_id) co_return;
	/* Delete the starboard message from the channel and the database (if found) */
	if (int64_t sb_msg_id = co_await cDatabase::SB_RemoveAll(message_id))
		co_await DeleteMessage(BOTS_CHANNEL_ID, sb_msg_id);
}