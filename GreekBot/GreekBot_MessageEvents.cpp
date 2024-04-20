#include "Database.h"
#include "GreekBot.h"
#include "Utils.h"
#include "CDN.h"

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
		/* Delete messages from the database and report to the log channel */
		for (auto& db_msg : co_await cDatabase::DeleteMessages(ids)) try {
			std::optional<cUser> user;
			try {
				user.emplace(co_await GetUser(db_msg.author_id));
			} catch (...) {}

			std::vector<cEmbed> embeds;
			cEmbed& embed = embeds.emplace_back();
			if (user) {
				auto disc = user->GetDiscriminator();
				embed.EmplaceAuthor(disc ? fmt::format("{}#{:04}", user->GetUsername(), disc) : user->MoveUsername()).SetIconUrl(cCDN::GetUserAvatar(*user));
			} else {
				embed.EmplaceAuthor("Deleted user").SetIconUrl(cCDN::GetDefaultUserAvatar(db_msg.author_id));
			}
			embed.SetDescription(fmt::format("❌ A message was **deleted** in <#{}>", db_msg.channel_id));
			embed.AddField(db_msg.content.empty() ? "No content" : "Content", db_msg.content);
			embed.SetColor(0xC43135);

			co_await CreateMessage(MESSAGE_LOG_CHANNEL_ID, kw::embeds=std::move(embeds));
		} catch (...) {}
		/* Delete the starboard messages from the channel and the database (if found) */
		for (cSnowflake& id : ids) {
			if (int64_t sb_msg_id = co_await cDatabase::SB_RemoveAll(id))
				co_await DeleteMessage(HOLY_CHANNEL_ID, sb_msg_id);
		}
	}
}