#include "CDN.h"
#include "GreekBot.h"
#include "DBMessageLog.h"

/* ========== Register when a new message is received =============================================================== */
cTask<>
cGreekBot::process_msglog_new_message(const cMessage& msg) HANDLER_BEGIN {
	/* Save message for logging purposes */
	auto txn = co_await BorrowDatabase();
	cMessageLogDAO(txn).Register(msg);
	co_await ReturnDatabase(std::move(txn));
} HANDLER_END
/* ========== Update message when its content changes and send confirmation message in the logging channel ========== */
cTask<>
cGreekBot::process_msglog_message_update(cMessageUpdate &msg) HANDLER_BEGIN {
	std::string& content = *msg.GetContent();
	/* Start a database transaction */
	auto txn = co_await BorrowDatabase();
	cMessageLogDAO dao(txn);
	txn.Begin();
	/* Retrieve the message from the database */
	std::optional db_msg = dao.Get(msg.GetId());
	/* If the message is found, update its content in the database */
	if (db_msg)
		dao.Update(db_msg->id, content);
	/* Commit the transaction */
	txn.Commit();
	co_await ReturnDatabase(std::move(txn));

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