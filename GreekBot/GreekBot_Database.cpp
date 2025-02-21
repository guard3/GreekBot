#include "Database.h"
#include "GreekBot.h"

static sqlite::connection g_db;

cTask<sqlite::connection>
cGreekBot::BorrowDatabase() {
	co_await cDatabase::ResumeOnDatabaseStrand();
	co_return g_db ? std::move(g_db) : cDatabase::CreateInstance();
}

cTask<>
cGreekBot::ReturnDatabase(sqlite::connection conn) {
	co_await cDatabase::ResumeOnDatabaseStrand();
	g_db = std::move(conn);
}

cTask<cTransaction>
cGreekBot::BorrowDatabaseTxn() {
	co_return cTransaction(co_await BorrowDatabase());
}

cTask<>
cGreekBot::ReturnDatabaseTxn(cTransaction txn) {
	co_await ReturnDatabase(txn.ReleaseConnection());
}