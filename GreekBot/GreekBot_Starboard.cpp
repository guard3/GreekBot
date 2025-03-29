#include "GreekBot.h"
#include "DBStarboard.h"
#include "Utils.h"
#include "CDN.h"
#include <ranges>

namespace rng = std::ranges;

static constexpr int REACTION_THRESHOLD = 5;

/* This array must be sorted for binary search to work */
static constexpr cSnowflake excluded_channels[] {
	350234736754688020,  // #rules
	355242373380308993,  // #moderators
	363441099307483139,  // #announcements
	366366855117668383,  // #controversial
	469274019565142017,  // #user-log
	486311040477298690,  // #word-of-the-day
	539521989061378048,  // #message-log
	595658812128624670,  // #deepest-lore
	598873442288271423,  // #contributors
	611889781227520003,  // #travel-and-meetups
	618350374348390406,  // #bulletin-board
	627082548706541568,  // #moderators 📣
	630486369495679007,  // #rules-welcome
	645650220440485927,  // #faq
	645650358042886175,  // #faq-welcome
	650331739293745172,  // #native-polls
	672924470750478338,  // #moderation-log
	817078394331856907,  // #private-discussions
	1138145632134115348, // #automod-channel
	1140760999331381260, // #temp-friendos 📣
	1143284169502367805, // #welcoming
	1143888492422770778  // #new-members
};
static_assert(rng::is_sorted(excluded_channels), "Must be sorted for binary search");

cTask<>
cGreekBot::OnMessageReactionAdd(cSnowflake& user_id, cSnowflake& channel_id, cSnowflake& message_id, hSnowflake guild_id, hSnowflake message_author_id, hMember member, cEmoji& emoji) {
	/* Make sure that we're in Learning Greek and that the emoji is :Holy: */
	if (!guild_id || !emoji.GetId())
		co_return;
	if (*guild_id != LMG_GUILD_ID || *emoji.GetId() != LMG_EMOJI_HOLY)
		co_return;
	/* Also make sure that we're not in an excluded channel */
	if (rng::binary_search(excluded_channels, channel_id))
		co_return;
	/* Also make sure that the message author id is provided */
	std::optional<cMessage> msg;
	if (!message_author_id) {
		auto& m = msg.emplace(co_await GetChannelMessage(channel_id, message_id));
		message_author_id = &m.GetAuthor().GetId();
	}
	/* Check that reactions from the author don't count */
	if (*message_author_id == user_id || *message_author_id == GetUser().GetId()) co_return;
	/* Register received reaction in the database */
	auto txn = co_await cDatabase::BorrowDatabase();
	auto[sb_msg_id, num_reactions] = cStarboardDAO(txn).RegisterReaction(message_id, *message_author_id);
	/* Process */
	co_await process_reaction(std::move(txn), channel_id, message_id, msg, sb_msg_id, num_reactions);
}

cTask<>
cGreekBot::OnMessageReactionRemove(cSnowflake& user_id, cSnowflake& channel_id, cSnowflake& message_id, hSnowflake guild_id, cEmoji& emoji) HANDLER_BEGIN {
	/* Make sure that we're in Learning Greek and that the emoji is :Holy: */
	if (!guild_id || !emoji.GetId()) co_return;
	if (*guild_id != LMG_GUILD_ID || *emoji.GetId() != LMG_EMOJI_HOLY) co_return;
	/* Also make sure that we're not in an excluded channel */
	if (rng::binary_search(excluded_channels, channel_id))
		co_return;

	auto txn = co_await cDatabase::BorrowDatabase();
	cStarboardDAO dao(txn);
	if (std::optional author_id = dao.GetMessageAuthor(message_id); !author_id || *author_id == user_id || *author_id == GetUser().GetId()) {
		/* Make sure that reactions from the author don't count */
		co_await cDatabase::ReturnDatabase(std::move(txn));
	} else {
		/* Remove one reaction from the database */
		auto[sb_msg_id, num_reactions] = dao.RemoveReaction(message_id);
		/* Process */
		std::optional<cMessage> msg;
		co_await process_reaction(std::move(txn), channel_id, message_id, msg, sb_msg_id, num_reactions);
	}
} HANDLER_END

cTask<>
cGreekBot::process_starboard_message_delete(std::span<const cSnowflake> msg_ids) HANDLER_BEGIN {
	/* Remove starboard entries and retrieve the ids of messages sent by the bot */
	auto txn = co_await cDatabase::BorrowDatabase();
	auto sb_msg_ids = cStarboardDAO(txn).DeleteAll(msg_ids);
	co_await cDatabase::ReturnDatabase(std::move(txn));
	/* Delete those messages */
	co_await DeleteMessages(LMG_CHANNEL_STARBOARD, sb_msg_ids);
} HANDLER_END

cTask<>
cGreekBot::process_reaction(cTransaction txn, crefChannel channel, crefMessage message, std::optional<cMessage>& msg, const std::optional<cSnowflake>& sb_msg_id, std::int64_t num_reactions) {
	/* If the number of reactions is less than the threshold, delete the starboard message if it was posted before */
	if (num_reactions < REACTION_THRESHOLD) {
		if (sb_msg_id) {
			cStarboardDAO(txn).RemoveMessage(message);
			co_await cDatabase::ReturnDatabase(std::move(txn));
			co_await DeleteMessage(LMG_CHANNEL_STARBOARD, *sb_msg_id);
		} else {
			co_await cDatabase::ReturnDatabase(std::move(txn));
		}
		co_return;
	}
	/* Otherwise, prepare the message content with the :Holy: count */
	std::string content = std::format("{} **{}** https://discord.com/channels/{}/{}/{}", [num_reactions] {
		switch (num_reactions) {
			case REACTION_THRESHOLD:
				return "<:Holy:409075809723219969>";
			case REACTION_THRESHOLD + 1:
				return "<:magik:1167594533849149450>";
			default:
				return "<a:spin:1167594572050866207>";
		}
	}(), num_reactions, LMG_GUILD_ID, channel.GetId(), message.GetId());
	/* If there is a message id registered in the database, edit the message with the new number of reactions */
	if (sb_msg_id) {
		co_await cDatabase::ReturnDatabase(std::move(txn));
		co_await EditMessage(LMG_CHANNEL_STARBOARD, *sb_msg_id, cMessageUpdate().SetContent(std::move(content)));
		co_return;
	}
	/* Make sure that we have the message object available */
	if (!msg)
		msg.emplace(co_await GetChannelMessage(channel, message));
	cMember author_member = co_await GetGuildMember(LMG_GUILD_ID, msg->GetAuthor().GetId());
	cUser&  author_user = *author_member.GetUser();
	/* Prepare the message response with a preview embed */
	cPartialMessage response;
	cEmbed& preview = response.EmplaceEmbeds().emplace_back();
	preview.EmplaceAuthor(author_user.MoveUsername()).SetIconUrl(cCDN::GetUserAvatar(author_user));
	preview.SetColor(get_lmg_member_color(author_member));
	preview.SetTimestamp(msg->GetTimestamp());
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
				preview.SetDescription(std::format("{}\n\n**Video**\n{}", msg->GetContent(), pVideo->GetUrl()));
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
	cMessage sb_msg = co_await CreateMessage(LMG_CHANNEL_STARBOARD, response.SetContent(std::move(content)));
	cStarboardDAO(txn).RegisterMessage(message, sb_msg);
	co_await cDatabase::ReturnDatabase(std::move(txn));
}

static cEmbed make_embed(const cUser& user, const starboard_entry& e, cColor color) {
	const char* medal;
	switch (e.rank) {
		case 1:  medal = "🥇"; break;
		case 2:  medal = "🥈"; break;
		case 3:  medal = "🥉"; break;
		default: medal = "🏅"; break;
	}

	cEmbed embed;
	embed.EmplaceAuthor(user.GetUsername()).SetIconUrl(cCDN::GetUserAvatar(user));
	embed.SetTitle(std::format("{} Rank **#{}**", medal, e.rank));
	embed.SetColor(color);
	embed.SetFields({
		{ "Total <:Holy:409075809723219969>", std::to_string(e.react_total), true },
		{ "Messages", std::to_string(e.num_msg), true },
		{ "Most in a single message", std::to_string(e.max_react_per_msg) }
	});
	return embed;
}

static cEmbed make_no_member_embed(const cUser* pUser, std::string_view guild_name, bool bAnymore) {
	cEmbed embed;
	embed.SetColor(LMG_COLOR_BLUE);
	embed.SetDescription(std::format("User is not a member of **{}**{}.", guild_name, bAnymore ? " anymore" : ""));
	if (pUser)
		embed.EmplaceAuthor(pUser->GetUsername()).SetIconUrl(cCDN::GetUserAvatar(*pUser));
	else
		embed.EmplaceAuthor("Deleted User").SetIconUrl(cCDN::GetDefaultUserAvatar(cSnowflake{}));
	return embed;
}

cTask<>
cGreekBot::process_starboard_leaderboard(cAppCmdInteraction& i) HANDLER_BEGIN {
	cPartialMessage response;
	auto& embeds = response.EmplaceEmbeds();
	/* Check which subcommand was invoked */
	switch (auto& subcommand = i.GetOptions().front(); cUtils::CRC32(0, subcommand.GetName())) {
		default: {
			struct _ : std::exception {
				const char* what() const noexcept override { return "Unknown subsommand"; }
			};
			throw _();
		}
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
			response.SetFlags(MESSAGE_FLAG_EPHEMERAL);
			if (user->IsBotUser())
				co_return co_await InteractionSendMessage(i, response.SetContent("Ranking isn't available for bot users."));
			if (user->IsSystemUser())
				co_return co_await InteractionSendMessage(i, response.SetContent("Ranking isn't available for system users."));
			response.ResetFlags();
			/* Acknowledge the interaction since we'll be accessing the database */
			co_await InteractionDefer(i);
			/* Retrieve user's starboard entry */
			auto txn = co_await cDatabase::BorrowDatabase();
			auto results = cStarboardDAO(txn).GetRank(*user, REACTION_THRESHOLD);
			co_await cDatabase::ReturnDatabase(std::move(txn));
			co_await ResumeOnEventStrand();
			/* If the user isn't a member of Learning Greek... */
			if (!member) {
				embeds.push_back(make_no_member_embed(user.Get(), m_guilds.at(LMG_GUILD_ID).GetName(), !results.empty()));
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
			auto& embed = embeds.emplace_back();
			embed.EmplaceAuthor(user->GetUsername()).SetIconUrl(cCDN::GetUserAvatar(*user));
			embed.SetColor(color);
			embed.SetDescription(std::format("User has no <:Holy:409075809723219969>ed messages. {}", msg));
			break;
		}
		case 0x1D400909: { // top10
			/* Acknowledge the interaction since we'll be accessing the database */
			co_await InteractionDefer(i);
			/* Retrieve the top 10 entries for starboard */
			auto txn = co_await cDatabase::BorrowDatabase();
			auto results = cStarboardDAO(txn).GetTop10(REACTION_THRESHOLD);
			co_await cDatabase::ReturnDatabase(std::move(txn));
			if (results.empty())
				co_return co_await InteractionSendMessage(i, response.SetContent("I have no <:Holy:409075809723219969> data yet. Y'all boring as fuck!"));
			/* Prepare member generator */
			std::vector<cMember> members; {
				auto user_ids = results | std::views::transform(&starboard_entry::author_id) | rng::to<std::vector>();
				members.reserve(results.size());
				co_for (auto& mem, RequestGuildMembers(*i.GetGuildId(), user_ids))
					members.push_back(std::move(mem));
			}
			co_await ResumeOnEventStrand();
			/* Get guild name in case it's needed */
			std::string guild_name = m_guilds.at(LMG_GUILD_ID).GetName();
			/* Create embeds */
			embeds.reserve(results.size());
			for (auto& entry : results) {
				if (auto it = rng::find(members, entry.author_id, [](auto& m) -> auto& { return m.GetUser()->GetId(); }); it != members.end()) {
					/* If the member is found, create an embed */
					embeds.push_back(make_embed(*it->GetUser(), entry, get_lmg_member_color(*it)));
				} else try {
					/* If there's no member object for a user, then this user isn't a member of Learning Greek anymore */
					cUser user = co_await GetUser(entry.author_id);
					embeds.push_back(make_no_member_embed(&user, guild_name, true));
				} catch (const xDiscordError&) {
					/* User object couldn't be retrieved, likely deleted */
					embeds.push_back(make_no_member_embed(nullptr, guild_name, true));
				}
			}
			break;
		}
	}
	/* Send the message with the created embeds */
	co_await InteractionSendMessage(i, response.SetComponents({
		cActionRow{
			cButton{
				BUTTON_STYLE_SECONDARY,
				"STARBOARD_HELP",
				"How does this work?"
			}
		}
	}));
} HANDLER_END

cTask<>
cGreekBot::process_starboard_help(cMsgCompInteraction& i) HANDLER_BEGIN {
	co_await InteractionSendMessage(i, cPartialMessage()
		.SetFlags(MESSAGE_FLAG_EPHEMERAL)
		.SetContent("When a message receives **5 or more** <:Holy:409075809723219969> reactions, it gets to appear in <#978993330694266920>. Reacting to *your own* messages doesn't count!")
	);
} HANDLER_END