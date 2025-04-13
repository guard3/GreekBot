#include "CDN.h"
#include "GreekBot.h"
#include "DBMessageLog.h"
#include <ranges>

/* ========== Register when a new message is received =============================================================== */
cTask<>
cGreekBot::process_msglog_new_message(const cMessage& msg) HANDLER_BEGIN {
	/* Save message for logging purposes */
	co_await cMessageLogDAO(co_await cDatabase::CreateTransaction()).Register(msg);
} HANDLER_END
/* ========== Update message when its content changes and send confirmation message in the logging channel ========== */
cTask<>
cGreekBot::process_msglog_message_update(cMessageUpdate &msg) HANDLER_BEGIN {
	std::string& content = *msg.GetContent();
	/* Start a database transaction */
	auto txn = co_await cDatabase::CreateTransaction();
	cMessageLogDAO dao(txn);
	co_await txn.Begin();
	/* Retrieve the message from the database */
	std::optional db_msg = co_await dao.Get(msg.GetId());
	/* If the message is found, update its content in the database */
	if (db_msg)
		co_await dao.Update(db_msg->id, content);
	/* Commit the transaction */
	co_await txn.Commit();
	txn.Close();

	/* If no message was found, there's nothing else to do */
	if (!db_msg)
		co_return;

	/* Prepare the response */
	cPartialMessage response;
	cEmbed& embed = response.EmplaceEmbeds().emplace_back();
	/* Retrieve the message author object to add an author to the embed */
	try {
		cUser author = co_await GetUser(db_msg->author_id);
		embed.EmplaceAuthor(author.MoveUsername()).SetIconUrl(cCDN::GetUserAvatar(author));
	} catch (const xDiscordError&) {
		/* User was not found but that's fine */
		embed.EmplaceAuthor("Deleted user").SetIconUrl(cCDN::GetDefaultUserAvatar(db_msg->author_id));
	}
	embed.SetColor(LMG_COLOR_LIGHT_GREEN);
	embed.SetDescription(std::format("📝 Their message was **edited** https://discord.com/channels/{}/{}/{}", LMG_GUILD_ID, db_msg->channel_id, db_msg->id));
	embed.SetTimestamp(db_msg->id.GetTimestamp());
	/* Add message id, user id and content changes as fields */
	auto& fields = embed.EmplaceFields();
	fields.reserve(4);
	fields.emplace_back("Message ID", std::format("`{}`", db_msg->id), true);
	fields.emplace_back("User ID", std::format("`{}`", db_msg->author_id), true);
	if (!db_msg->content.empty())
		fields.emplace_back("Old content", std::move(db_msg->content));
	fields.emplace_back(content.empty() ? "No new content" : "New content", std::move(content));

	/* Send the final message to the log channel */
	co_await CreateMessage(LMG_CHANNEL_MESSAGE_LOG, response);
} HANDLER_END
/* ========== Report deleted messages =============================================================================== */
cTask<>
cGreekBot::process_msglog_message_delete(std::span<const cSnowflake> msg_ids) HANDLER_BEGIN {
	/* Delete messages from the database */
	std::vector db_msgs = co_await cMessageLogDAO(co_await cDatabase::CreateTransaction()).Delete(msg_ids);
	/* Retrieve the authors of the deleted messages */
	std::vector<std::variant<cUser, cSnowflake>> authors; {
		auto user_ids = db_msgs | std::views::transform(&message_entry::author_id) | std::ranges::to<std::vector>();
		co_for (cMember& member, RequestGuildMembers(LMG_GUILD_ID, user_ids))
			authors.emplace_back(std::move(*member.GetUser()));
	}
	/* Prepare the embed vector for the log messages */
	cPartialMessage response;
	auto& embeds = response.EmplaceEmbeds();
	embeds.reserve(10);
	for (auto& db_msg : db_msgs) {
		const cUser* pUser = nullptr;
		/* Search the user vector for the current message author */
		if (auto it = std::ranges::find(authors, db_msg.author_id, [](const auto& var) -> const auto& {
			return std::visit(cVisitor{
				[](const cSnowflake& id) -> const auto& { return id; },
				[](const cUser& user) -> const auto& { return user.GetId(); }
			}, var);
		}); it != authors.end()) {
			/* User found, although not necessarily as a complete cUser object */
			pUser = std::get_if<cUser>(&*it);
		} else try {
			/* User object not found, try to retrieve it */
			pUser = std::get_if<cUser>(&authors.emplace_back(co_await GetUser(db_msg.author_id)));
		} catch (const xDiscordError&) {
			/* User not found, just save the id to indicate that we've tested for it before */
			authors.emplace_back(db_msg.author_id);
		}
		/* Create the embed */
		cEmbed& embed = embeds.emplace_back();
		if (pUser)
			embed.EmplaceAuthor(pUser->GetUsername()).SetIconUrl(cCDN::GetUserAvatar(*pUser));
		else
			embed.EmplaceAuthor("Deleted user").SetIconUrl(cCDN::GetDefaultUserAvatar(db_msg.author_id));
		embed.SetColor(LMG_COLOR_RED);
		embed.SetDescription(std::format("❌ Their message was **deleted** in <#{}>", db_msg.channel_id));
		embed.SetTimestamp(db_msg.id.GetTimestamp());
		embed.SetFields({
			{ "Message ID", std::format("`{}`", db_msg.id), true },
			{ "User ID", std::format("`{}`", db_msg.author_id), true },
			{ db_msg.content.empty() ? "No content" : "Content", std::move(db_msg.content) }
		});
		/* If we reach the maximum amount of embeds supported per message, send them */
		if (embeds.size() == 10) {
			co_await CreateMessage(LMG_CHANNEL_MESSAGE_LOG, response);
			embeds.clear();
		}
	}
	/* Send any remaining embeds */
	if (!embeds.empty())
		co_await CreateMessage(LMG_CHANNEL_MESSAGE_LOG, response);
} HANDLER_END