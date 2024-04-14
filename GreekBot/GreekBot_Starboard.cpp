#include "GreekBot.h"
#include "Database.h"
#include "Utils.h"
#include "CDN.h"

static constexpr int REACTION_THRESHOLD = 5;

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
	co_await ResumeOnEventThread();
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
			kw::icon_url=cCDN::GetUserAvatar(author_user)
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
				if (pVideo) break;
			}
			if (content_type.starts_with("video/")) {
				if (pVideo) continue;
				pVideo = &a;
				if (pImage) break;
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

static cEmbed make_embed(const cUser& user, const starboard_entry& e, cColor color) {
	const char* medal;
	switch (e.rank) {
		case 1:  medal = "🥇"; break;
		case 2:  medal = "🥈"; break;
		case 3:  medal = "🥉"; break;
		default: medal = "🏅"; break;
	}

	return cEmbed{
		kw::author={
			user.GetUsername(),
			kw::icon_url=cCDN::GetUserAvatar(user)
		},
		kw::title=fmt::format("{} Rank#{}", medal, e.rank),
		kw::color=color,
		kw::fields={
			{ "Total <:Holy:409075809723219969>", std::to_string(e.react_total), true },
			{ "Messages", std::to_string(e.num_msg), true },
			{ "Most in a single message", std::to_string(e.max_react_per_msg) }
		}
	};
}

static cEmbed make_no_member_embed(const cUser* pUser, std::string_view guild_name, bool bAnymore) {
	return cEmbed{
		kw::author=pUser ? cEmbedAuthor{
				pUser->GetUsername(),
				kw::icon_url=cCDN::GetUserAvatar(*pUser)
			} : cEmbedAuthor{
				"Deleted User",
				kw::icon_url=cCDN::GetDefaultUserAvatar(cSnowflake())
			},
		kw::color=0x0096FF,
		kw::description=fmt::format("User is not a member of **{}**{}.", guild_name, bAnymore ? " anymore" : "")
	};
}

cTask<>
cGreekBot::process_starboard_leaderboard(cAppCmdInteraction& i) HANDLER_BEGIN {
	std::vector<cEmbed> embeds;
	/* Check which subcommand was invoked */
	switch (auto& subcommand = i.GetOptions().front(); cUtils::CRC32(0, subcommand.GetName())) {
		default:
			throw std::runtime_error("Unknown subcommand");
		case 0x8879E8E5: { // rank
			/* Retrieve selected member and user */
			hUser user;
			hPartialMember member;
			if (auto options = subcommand.GetOptions(); !options.empty()) {
				std::tie(user, member) = options.front().GetValue<APP_CMD_OPT_USER>();
			} else {
				user = &i.GetUser();
				member = i.GetMember();
			}
			/* Limit the command to regular users */
			if (user->IsBotUser())
				co_return co_await InteractionSendMessage(i, cMessageParams{
					kw::flags=MESSAGE_FLAG_EPHEMERAL,
					kw::content="Ranking isn't available for bot users."
				});
			if (user->IsSystemUser())
				co_return co_await InteractionSendMessage(i, cMessageParams{
					kw::flags=MESSAGE_FLAG_EPHEMERAL,
					kw::content="Ranking isn't available for system users."
				});
			/* Acknowledge the interaction since we'll be accessing the database */
			co_await InteractionDefer(i);
			/* Retrieve user's starboard entry */
			auto results = co_await cDatabase::SB_GetRank(*user, REACTION_THRESHOLD);
			co_await ResumeOnEventThread();
			/* If the user isn't a member of Learning Greek... */
			if (!member) {
				embeds.push_back(make_no_member_embed(user.Get(), m_guilds.at(m_lmg_id)->GetName(), !results.empty()));
				break;
			}
			cColor color = get_lmg_member_color(*member);
			/* If the user is registered in the leaderboard... */
			if (!results.empty()) {
				embeds.push_back(make_embed(*user, results.front(), color));
				break;
			}
			/* Otherwise... */
			const char *msg;
			switch (cUtils::Random(0, 3)) {
				case 0:  msg = "Booooring!"; break;
				case 1:  msg = "SAD!";       break;
				case 2:  msg = "Meh...";     break;
				default: msg = "Laaaaame!";  break;
			}
			embeds.emplace_back(
				kw::author = {
					user->GetUsername(),
					kw::icon_url = cCDN::GetUserAvatar(*user)
				},
				kw::color = color,
				kw::description = fmt::format("User has no <:Holy:409075809723219969>ed messages. {}", msg)
			);
			break;
		}
		case 0x1D400909: { // top10
			/* Acknowledge the interaction since we'll be accessing the database */
			co_await InteractionDefer(i);
			/* Retrieve the top 10 entries for starboard */
			auto results = co_await cDatabase::SB_GetTop10(REACTION_THRESHOLD);
			co_await ResumeOnEventThread();
			if (results.empty())
				co_return co_await InteractionSendMessage(i, cMessageParams{
					kw::content="I have no <:Holy:409075809723219969> data yet. Y'all boring as fuck!"
				});
			/* Retrieve members */
			std::vector<cMember> members;
			{
				std::vector<cSnowflake> ids;
				ids.reserve(10);
				for (auto &e: results)
					ids.push_back(e.author_id);
				auto gen = GetGuildMembers(m_lmg_id, kw::user_ids=std::move(ids));
				members.reserve(10);
				while (co_await gen.HasValue())
					members.push_back(co_await gen.Next());
			}
			/* Create embeds */
			embeds.reserve(10);
			for (auto& entry : results) {
				auto it = std::find_if(members.begin(), members.end(), [&entry](const cMember& m) {
					return m.GetUser()->GetId() == entry.author_id;
				});
				if (it == members.end()) {
					/* If there's no member object for a user, then this user isn't a member of Learning Greek anymore */
					auto& guild_name = m_guilds.at(m_lmg_id)->GetName();
					try {
						cUser user = co_await GetUser(entry.author_id);
						embeds.push_back(make_no_member_embed(&user, guild_name, true));
					} catch (const xDiscordError&) {
						/* User object couldn't be retrieved, likely deleted */
						embeds.push_back(make_no_member_embed(nullptr, guild_name, true));
					}
				} else {
					/* If the member is found, create an embed */
					embeds.push_back(make_embed(*it->GetUser(), entry, get_lmg_member_color(*it)));
				}
			}
			break;
		}
	}
	/* Send the message with the created embeds */
	co_await InteractionSendMessage(i, cMessageParams{
		kw::embeds=std::move(embeds),
		kw::components={
			cActionRow{
				cButton{
					BUTTON_STYLE_SECONDARY,
					"STARBOARD_HELP",
					kw::label="How does this work?"
				}
			}
		}
	});
} HANDLER_END

cTask<>
cGreekBot::process_starboard_help(cMsgCompInteraction& i) HANDLER_BEGIN {
	co_await InteractionSendMessage(i, cMessageParams{
		kw::flags=MESSAGE_FLAG_EPHEMERAL,
		kw::content="When a message receives **5 or more** <:Holy:409075809723219969> reactions, it gets to appear in <#978993330694266920>. Reacting to *your own* messages doesn't count!"
	});
} HANDLER_END