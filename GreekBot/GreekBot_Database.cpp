#include "Database.h"
#include "GreekBot.h"

static sqlite::connection g_db;

cTask<cTransaction>
cGreekBot::BorrowDatabase() {
	co_await cDatabase::ResumeOnDatabaseStrand();
	co_return cTransaction(g_db ? std::move(g_db) : cDatabase::CreateInstance());
}

cTask<>
cGreekBot::ReturnDatabase(cTransaction txn) {
	co_await cDatabase::ResumeOnDatabaseStrand();
	g_db = txn.ReleaseConnection();
}