#include "Database.h"
#include "GreekBot.h"
#include "CDN.h"
#include "Utils.h"
#include <algorithm>
#include <ranges>
#include <variant>

namespace rng = std::ranges;

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

static void
add_message_delete_embed(std::vector<cEmbed>& embeds, const message_entry& db_msg, const cUser* pMsgAuthor) {
	cEmbed& embed = embeds.emplace_back();
	embed.SetColor(LMG_COLOR_RED);
	embed.SetDescription(std::format("❌ Their message was **deleted** in <#{}>", db_msg.channel_id));
	embed.SetTimestamp(db_msg.id.GetTimestamp());
	embed.SetFields({
		{ "Message ID", std::format("`{}`", db_msg.id.ToString()), true },
		{ "User ID", std::format("`{}`", db_msg.author_id), true },
		{ db_msg.content.empty() ? "No content" : "Content", db_msg.content }
	});
	if (pMsgAuthor) {
		embed.EmplaceAuthor(pMsgAuthor->GetUsername()).SetIconUrl(cCDN::GetUserAvatar(*pMsgAuthor));
	} else {
		embed.EmplaceAuthor("Deleted user").SetIconUrl(cCDN::GetDefaultUserAvatar(db_msg.author_id));
	}
}

cTask<>
cGreekBot::OnMessageDelete(cSnowflake& message_id, cSnowflake& channel_id, hSnowflake guild_id) {
	/* Make sure we're in Learning Greek */
	if (guild_id && *guild_id == LMG_GUILD_ID) {
		/* Delete message from the database and report to the log channel */
		if (auto db_msg = co_await cDatabase::DeleteMessage(message_id)) try {
			std::optional<cUser> user;
			const cUser* pUser{};
			try {
				pUser = &user.emplace(co_await GetUser(db_msg->author_id));
			} catch (...) {}

			cPartialMessage response;
			add_message_delete_embed(response.EmplaceEmbeds(), *db_msg, pUser);
			co_await CreateMessage(LMG_CHANNEL_MESSAGE_LOG, response);
		} catch (...) {}
		/* Delete the starboard message from the channel and the database (if found) */
		if (int64_t sb_msg_id = co_await cDatabase::SB_RemoveAll(message_id))
			co_await DeleteMessage(LMG_CHANNEL_STARBOARD, sb_msg_id);
	}
}
cTask<>
cGreekBot::OnMessageDeleteBulk(std::span<cSnowflake> ids, cSnowflake& channel_id, hSnowflake guild_id) {
	/* Make sure we're in Learning Greek */
	if (guild_id && *guild_id == LMG_GUILD_ID) {
		try {
			/* Delete messages from the database */
			auto db_msgs = co_await cDatabase::DeleteMessages(ids);
			/* Retrieve the authors of the deleted messages */
			auto members = co_await [this, guild_id, &db_msgs]() -> cTask<std::vector<std::variant<cUser, cSnowflake>>> {
				const auto size = db_msgs.size();
				std::vector<cSnowflake> user_ids;
				user_ids.reserve(size);
				for (auto& db_msg : db_msgs)
					user_ids.push_back(db_msg.author_id);
				std::vector<std::variant<cUser, cSnowflake>> members;
				members.reserve(size);
				co_for(auto& mem, RequestGuildMembers(*guild_id, user_ids))
					members.emplace_back(std::move(*mem.GetUser()));
				co_return members;
			}();
			/* Prepare the embed vector for the log messages */
			cPartialMessage response;
			auto& embeds = response.EmplaceEmbeds();
			embeds.reserve(10);
			for (auto& db_msg : db_msgs) {
				/* Retrieve the user object of the message author */
				const cUser* pUser = nullptr;
				for (auto it = members.begin();; ++it) {
					if (it == members.end()) {
						try {
							pUser = &std::get<cUser>(members.emplace_back(co_await GetUser(db_msg.author_id)));
						} catch (const xDiscordError&) {
							members.emplace_back(db_msg.author_id);
						}
						break;
					}
					auto& var = *it;
					if (auto user = std::get_if<cUser>(&var); db_msg.author_id == (user ? user->GetId() : std::get<cSnowflake>(var))) {
						pUser = user;
						break;
					}
				}
				/* Create the embed */
				add_message_delete_embed(embeds, db_msg, pUser);
				/* If we reach the maximum amount of embeds supported per message, send them */
				if (embeds.size() == 10) {
					co_await CreateMessage(LMG_CHANNEL_MESSAGE_LOG, response);
					embeds.clear();
				}
			}
			/* Send any remaining embeds */
			if (!embeds.empty())
				co_await CreateMessage(LMG_CHANNEL_MESSAGE_LOG, response);
		} catch (const std::exception& e) {
			cUtils::PrintErr("An error occurred while reporting deleted messages: {}", e.what());
		}
		/* Delete the starboard messages from the channel and the database (if found) */
		for (cSnowflake& id : ids) {
			if (int64_t sb_msg_id = co_await cDatabase::SB_RemoveAll(id)) try {
				co_await DeleteMessage(LMG_CHANNEL_STARBOARD, sb_msg_id);
			} catch (...) {}
		}
	}
}