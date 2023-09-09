#include "Database.h"
#include "Queries.h"
#include "Utils.h"
#include <sqlite3.h>
#include <deque>
#include <filesystem>
#include <mutex>
#include <thread>

namespace fs = std::filesystem;

#if   defined _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <Windows.h>
#  include <Shlwapi.h>
#elif defined __APPLE__
#  include <mach-o/dyld.h>
#endif

static sqlite3* g_db = nullptr;
static std::deque<std::function<void()>> g_queue;
static std::thread g_thread;
static std::mutex g_mutex;
cDatabase cDatabase::ms_instance;

xDatabaseError::xDatabaseError() : std::runtime_error(sqlite3_errmsg(g_db)), m_code(sqlite3_extended_errcode(g_db)) {}

/* Get a fully qualified UTF-8 path for the database file */
static std::u8string get_db_filename() {
#if defined(_WIN32)
#elif defined(__APPLE__)
	std::vector<char> path(256);
	if (uint32_t size = 256; _NSGetExecutablePath(path.data(), &size) < 0) {
		path.resize(size);
		_NSGetExecutablePath(path.data(), &size);
	}
	fs::path result = fs::canonical(path.data());
#else
	fs::path result = fs::canonical("/proc/self/exe");
#endif
	return result.replace_filename("database.db").u8string();
}

cDatabase::cDatabase() {
	sqlite3 *db = nullptr; // The database connection handle
	std::string err_msg;   // The error message string
	try {
		/* Open a database connection */
		constexpr int DB_FLAGS = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
		int rc = sqlite3_open_v2((const char*)get_db_filename().c_str(), &db, DB_FLAGS, nullptr);
		/* If db is null after opening, a memory error occurred */
		if (!db) throw std::bad_alloc();
		/* If database handle is open, execute the init statements */
		if (rc == SQLITE_OK) {
			sqlite3_stmt *stmt = nullptr;
			for (const char *query = QUERY_INIT, *end; SQLITE_OK == sqlite3_prepare_v2(db, query, -1, &stmt, &end); query = end) {
				if (stmt) {
					int status = sqlite3_step(stmt);
					sqlite3_finalize(stmt);
					if (status != SQLITE_DONE)
						break;
				} else {
					g_db = db;
					cUtils::PrintLog("Database initialized successfully");
					return;
				}
			}
		}
		/* At this point, initialization was unsuccessful */
		err_msg = sqlite3_errmsg(db);
	} catch (const std::exception& e) {
		err_msg = e.what();
	} catch (...) {
		err_msg = "An exception was thrown";
	}
	sqlite3_close(db);
	cUtils::PrintErr("Database initialization error: {}", err_msg);
	exit(EXIT_FAILURE);
}

cDatabase::~cDatabase() {
	/* Wait for all tasks to complete */
	if (g_thread.joinable())
		g_thread.join();
	/* Close the database connection */
	sqlite3_close(g_db);
}

void cDatabaseTask<void>::await_suspend(std::coroutine_handle<> h) {
	{
		/* Add current task to the queue */
		std::lock_guard _(g_mutex);
		g_queue.emplace_back([this, h] { m_func(h); });
		/* If there's more than one tasks, return and let the queue be consumed */
		if (g_queue.size() > 1) return;
	}
	if (g_thread.joinable()) g_thread.join();
	g_thread = std::thread([] {
		for (;;) {
			g_queue.front()();
			std::lock_guard _(g_mutex);
			g_queue.pop_front();
			if (g_queue.empty())
				break;
		}
	});
}

cDatabaseTask<>
cDatabase::UpdateLeaderboard(const cMessage& msg) {
	return [&msg]() {
		using namespace std::chrono;
		/* Execute QUERY_UPDATE_LB */
		sqlite3_stmt* stmt;
		if (SQLITE_OK == sqlite3_prepare_v2(g_db, QUERY_UPDATE_LB, sizeof(QUERY_UPDATE_LB), &stmt, nullptr)) {
			if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, msg.GetAuthor().GetId().ToInt())) {
				// TODO: update db and save discord epoch milliseconds directly
				if (SQLITE_OK == sqlite3_bind_int64(stmt, 2, duration_cast<seconds>(cDiscordClock::to_sys(msg.GetId().GetTimestamp()).time_since_epoch()).count())) {
					if (SQLITE_DONE == sqlite3_step(stmt)) {
						sqlite3_finalize(stmt);
						return;
					}
				}
			}
		}
		sqlite3_finalize(stmt);
		throw xDatabaseError();
	};
}

cDatabaseTask<uint64_t>
cDatabase::WC_RegisterMember(const cMember& member) {
	return [&member]() -> uint64_t {
		sqlite3_stmt* stmt = nullptr;
		if (SQLITE_OK == sqlite3_prepare_v2(g_db, QUERY_WC_REG_MBR, sizeof(QUERY_WC_REG_MBR), &stmt, nullptr)) {
			if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, member.GetUser()->GetId().ToInt())) {
				if (SQLITE_OK == sqlite3_bind_int64(stmt, 2, member.JoinedAt().time_since_epoch().count())) {
					if (SQLITE_ROW == sqlite3_step(stmt)) {
						int64_t result = sqlite3_column_int64(stmt, 0);
						sqlite3_finalize(stmt);
						return static_cast<uint64_t>(result);
					}
				}
			}
		}
		sqlite3_finalize(stmt);
		throw xDatabaseError();
	};
}

cDatabaseTask<int64_t>
cDatabase::WC_GetMessage(const cPartialMember& member) {
	return [&member]() -> int64_t {
		sqlite3_stmt* stmt = nullptr;
		if (SQLITE_OK == sqlite3_prepare_v2(g_db, QUERY_WC_GET_MSG, sizeof(QUERY_WC_GET_MSG), &stmt, nullptr)) {
			if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, member.GetUser().GetId().ToInt())) {
				switch (int64_t result; sqlite3_step(stmt)) {
					case SQLITE_DONE:
						result = -1;
						goto LABEL_DONE;
					case SQLITE_ROW:
						result = sqlite3_column_int64(stmt, 0);
					LABEL_DONE:
						sqlite3_finalize(stmt);
						return result;
					default:
						break;
				}
			}
		}
		sqlite3_finalize(stmt);
		throw xDatabaseError();
	};
}

cDatabaseTask<>
cDatabase::WC_EditMessage(int64_t msg_id) {
	return [msg_id]() {
		sqlite3_stmt* stmt = nullptr;
		if (SQLITE_OK == sqlite3_prepare_v2(g_db, QUERY_WC_EDT_MSG, sizeof(QUERY_WC_EDT_MSG), &stmt, nullptr)) {
			if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, msg_id)) {
				if (SQLITE_DONE == sqlite3_step(stmt)) {
					sqlite3_finalize(stmt);
					return;
				}
			}
		}
		sqlite3_finalize(stmt);
		throw xDatabaseError();
	};
}

cDatabaseTask<uint64_t>
cDatabase::WC_DeleteMember(const cUser& user) {
	return [&user]() {
		sqlite3_stmt* stmt = nullptr;
		if (SQLITE_OK == sqlite3_prepare_v2(g_db, QUERY_WC_DEL_MBR, sizeof(QUERY_WC_DEL_MBR), &stmt, nullptr)) {
			if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, user.GetId().ToInt())) {
				switch (uint64_t result; sqlite3_step(stmt)) {
					case SQLITE_DONE:
						result = 0;
						goto LABEL_DONE;
					case SQLITE_ROW:
						result = static_cast<uint64_t>(sqlite3_column_int64(stmt, 0));
					LABEL_DONE:
						sqlite3_finalize(stmt);
						return result;
				}
			}
		}
		sqlite3_finalize(stmt);
		throw xDatabaseError();
	};
}

cDatabaseTask<>
cDatabase::WC_UpdateMessage(const cUser& user, const cMessage& msg) {
	return [&user, &msg]() {
		sqlite3_stmt* stmt = nullptr;
		if (SQLITE_OK == sqlite3_prepare_v2(g_db, QUERY_WC_UPD_MSG, sizeof(QUERY_WC_UPD_MSG), &stmt, nullptr)) {
			if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, user.GetId().ToInt())) {
				if (SQLITE_OK == sqlite3_bind_int64(stmt, 2, msg.GetId().ToInt())) {
					if (SQLITE_DONE == sqlite3_step(stmt)) {
						sqlite3_finalize(stmt);
						return;
					}
				}
			}
		}
		sqlite3_finalize(stmt);
		throw xDatabaseError();
	};
}

cDatabaseTask<tRankQueryData>
cDatabase::GetUserRank(const cUser& user) {
	return [&user]() {
		sqlite3_stmt* stmt;
		tRankQueryData res;
		if (SQLITE_OK == sqlite3_prepare_v2(g_db, QUERY_GET_RANK, sizeof(QUERY_GET_RANK), &stmt, nullptr)) {
			if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, user.GetId().ToInt())) {
				switch (sqlite3_step(stmt)) {
					case SQLITE_ROW:
						/* Statement returned data for the user */
						res.emplace_back(
							sqlite3_column_int64(stmt, 0),
							user.GetId(),
							sqlite3_column_int64(stmt, 1),
							sqlite3_column_int64(stmt, 2)
						);
					case SQLITE_DONE:
						/* Statement didn't return, no user data found */
						sqlite3_finalize(stmt);
						return res;
				}
			}
		}
		sqlite3_finalize(stmt);
		throw xDatabaseError();
	};
}

cDatabaseTask<tRankQueryData>
cDatabase::GetTop10() {
	return []() {
		int rc;
		tRankQueryData res;
		sqlite3_stmt* stmt;
		if (SQLITE_OK == (rc = sqlite3_prepare_v2(g_db, QUERY_GET_TOP_10, sizeof(QUERY_GET_TOP_10), &stmt, nullptr))) {
			res.reserve(10);
			while (SQLITE_ROW == (rc = sqlite3_step(stmt))) {
				res.emplace_back(
					sqlite3_column_int64(stmt, 0),
					sqlite3_column_int64(stmt, 1),
					sqlite3_column_int64(stmt, 2),
					sqlite3_column_int64(stmt, 3)
				);
			}
		}
		sqlite3_finalize(stmt);
		if (rc == SQLITE_DONE)
			return res;
		throw xDatabaseError();
	};
}