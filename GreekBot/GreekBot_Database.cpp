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