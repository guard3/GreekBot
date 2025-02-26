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