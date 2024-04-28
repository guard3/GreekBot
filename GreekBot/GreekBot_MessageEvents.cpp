#include "Database.h"
#include "GreekBot.h"
#include "Utils.h"
#include "CDN.h"
#include <algorithm>

static const cSnowflake MESSAGE_LOG_CHANNEL_ID = 539521989061378048;

cTask<>
cGreekBot::OnMessageCreate(cMessage& msg, hSnowflake guild_id, hPartialMember member) {
	/* Make sure we're in Learning Greek and that the message author is a real user */
	if (cUser& author = msg.GetAuthor(); guild_id && *guild_id == m_lmg_id && !author.IsBotUser() && !author.IsSystemUser()) {
		/* Save message for logging purposes */
		co_await cDatabase::RegisterMessage(msg);
		/* Update leaderboard */
		co_await cDatabase::UpdateLeaderboard(msg);
		/* Cleanup old logged messages in 1 hour intervals */
		using namespace std::chrono;
		using namespace std::chrono_literals;
		if (auto now = steady_clock::now(); now - m_before > 1h) {
			m_before = now;
			co_await cDatabase::CleanupMessages();
		}
	}
}
cTask<>
cGreekBot::OnMessageUpdate(cMessageUpdate& msg, hSnowflake guild_id, hPartialMember member) {
	if (auto pContent = msg.GetContent(); guild_id && *guild_id == LMG_GUILD_ID && pContent) {
		if (auto db_msg = co_await cDatabase::UpdateMessage(msg.GetId(), *pContent)) {
			std::optional<cUser> user;
			try {
				user.emplace(co_await GetUser(db_msg->author_id));
			} catch (...) {}

			std::vector<cEmbed> embeds;
			cEmbed& embed = embeds.emplace_back();
			if (user) {
				auto disc = user->GetDiscriminator();
				embed.EmplaceAuthor(disc ? fmt::format("{}#{:04}", user->GetUsername(), disc) : user->MoveUsername()).SetIconUrl(cCDN::GetUserAvatar(*user));
			} else {
				embed.EmplaceAuthor("Deleted user").SetIconUrl(cCDN::GetDefaultUserAvatar(db_msg->author_id));
			}
			embed.SetDescription(fmt::format("📝 A message was **edited** in <#{}>", db_msg->channel_id));
			if (!db_msg->content.empty())
				embed.AddField("Old content", db_msg->content);
			embed.AddField(pContent->empty() ? "No new content" : "New content", *pContent);
			embed.SetColor(0x2ECD72);

			co_await CreateMessage(MESSAGE_LOG_CHANNEL_ID, kw::embeds=std::move(embeds));
		}
	}
}
cTask<>
cGreekBot::OnMessageDelete(cSnowflake& message_id, cSnowflake& channel_id, hSnowflake guild_id) {
	/* Make sure we're in Learning Greek */
	if (guild_id && *guild_id == m_lmg_id) {
		/* Delete message from the database and report to the log channel */
		if (auto db_msg = co_await cDatabase::DeleteMessage(message_id)) try {
			std::optional<cUser> user;
			try {
				user.emplace(co_await GetUser(db_msg->author_id));
			} catch (...) {}

			std::vector<cEmbed> embeds;
			cEmbed& embed = embeds.emplace_back();
			if (user) {
				auto disc = user->GetDiscriminator();
				embed.EmplaceAuthor(disc ? fmt::format("{}#{:04}", user->GetUsername(), disc) : user->MoveUsername()).SetIconUrl(cCDN::GetUserAvatar(*user));
			} else {
				embed.EmplaceAuthor("Deleted user").SetIconUrl(cCDN::GetDefaultUserAvatar(db_msg->author_id));
			}
			embed.SetDescription(fmt::format("❌ A message was **deleted** in <#{}>", db_msg->channel_id));
			embed.AddField(db_msg->content.empty() ? "No content" : "Content", db_msg->content);
			embed.SetColor(0xC43135);

			co_await CreateMessage(MESSAGE_LOG_CHANNEL_ID, kw::embeds=std::move(embeds));
		} catch (...) {}
		/* Delete the starboard message from the channel and the database (if found) */
		if (int64_t sb_msg_id = co_await cDatabase::SB_RemoveAll(message_id))
			co_await DeleteMessage(HOLY_CHANNEL_ID, sb_msg_id);
	}
}
cTask<>
cGreekBot::OnMessageDeleteBulk(std::span<cSnowflake> ids, cSnowflake& channel_id, hSnowflake guild_id) {
	/* Make sure we're in Learning Greek */
	if (guild_id && *guild_id == m_lmg_id) {
		try {
			/* Delete messages from the database */
			auto db_msgs = co_await cDatabase::DeleteMessages(ids);
			/* Retrieve the authors of the deleted messages */
			std::vector<cMember> members;
			members.reserve(ids.size());
			std::vector<cUser> non_members;
			non_members.reserve(ids.size());
			std::vector<cSnowflake> non_users;
			non_users.reserve(ids.size());
			{
				std::vector<cSnowflake> author_ids;
				author_ids.reserve(ids.size());
				for (auto& db_msg : db_msgs)
					author_ids.push_back(db_msg.author_id);
				auto gen = GetGuildMembers(LMG_GUILD_ID, kw::user_ids=std::move(author_ids));
				for (auto it = co_await gen.begin(); it != gen.end(); co_await ++it)
					members.push_back(std::move(*it));
			}
			/* Prepare the embed vector for the log messages */
			std::vector<cEmbed> embeds;
			embeds.reserve(10);
			for (auto& db_msg : db_msgs) {
				/* Retrieve the user object of the message author */
				hUser pUser{};
				if (auto it = std::ranges::find_if(members, [id = db_msg.author_id](const cMember& member) {
					return member.GetUser()->GetId() == id;
				}); it != std::ranges::end(members)) {
					pUser = it->GetUser();
				} else if (auto it = std::ranges::find_if(non_members, [id = db_msg.author_id](const cUser& user) {
					return user.GetId() == id;
				}); it != std::ranges::end(non_members)) {
					pUser = &*it;
				} else if (auto it = std::ranges::find(non_users, db_msg.author_id); it == std::ranges::end(non_users)) try {
					pUser = &non_members.emplace_back(co_await GetUser(db_msg.author_id));
				} catch (...) {
					non_users.push_back(db_msg.author_id);
				}
				/* Create the embed */
				cEmbed& embed = embeds.emplace_back();
				if (pUser) {
					auto disc = pUser->GetDiscriminator();
					embed.EmplaceAuthor(disc ? fmt::format("{}#{:04}", pUser->GetUsername(), disc) : (std::string)pUser->GetUsername()).SetIconUrl(cCDN::GetUserAvatar(*pUser));
				} else {
					embed.EmplaceAuthor("Deleted user").SetIconUrl(cCDN::GetDefaultUserAvatar(db_msg.author_id));
				}
				embed.SetDescription(fmt::format("❌ A message was **deleted** in <#{}>", db_msg.channel_id));
				embed.AddField(db_msg.content.empty() ? "No content" : "Content", db_msg.content);
				embed.SetColor(0xC43135);
				/* If we reach the maximum amount of embeds supported per message, send them */
				if (embeds.size() == 10) {
					co_await CreateMessage(MESSAGE_LOG_CHANNEL_ID, kw::embeds=std::move(embeds));
					embeds.clear();
					embeds.reserve(10);
				}
			}
			/* Send any remaining embeds */
			if (!embeds.empty())
				co_await CreateMessage(MESSAGE_LOG_CHANNEL_ID, kw::embeds=std::move(embeds));
		} catch (...) {}
		/* Delete the starboard messages from the channel and the database (if found) */
		for (cSnowflake& id : ids) {
			if (int64_t sb_msg_id = co_await cDatabase::SB_RemoveAll(id))
				co_await DeleteMessage(HOLY_CHANNEL_ID, sb_msg_id);
		}
	}
}