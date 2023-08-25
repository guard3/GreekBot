#include "Database.h"
#include "Queries.h"
#include "Utils.h"
#include <cstdlib>
#include <sqlite3.h>
#include <thread>
#include <mutex>
#include <deque>

/* The filename of the database */
#define DB_FILENAME "database.db"

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
	#include <Shlwapi.h>
#else
	#include <climits>
	#include <cstring>
	#ifdef __APPLE__
		#include <mach-o/dyld.h>
	#else
		#include <unistd.h>
	#endif
#endif

cDatabase cDatabase::ms_instance;
static sqlite3* g_db;
static std::deque<std::function<void()>> g_queue;
static std::thread g_thread;
static std::mutex g_mutex;

xDatabaseError::xDatabaseError() : std::runtime_error(sqlite3_errmsg(g_db)), m_code(sqlite3_extended_errcode(g_db)) {}

cDatabase::cDatabase() {
	/* Get the path of the database file */
	char* db_filename = nullptr;
#ifdef _WIN32
	DWORD dwSize = MAX_PATH;
	for (LPWSTR p; (p = (LPWSTR)realloc(db_filename, dwSize * sizeof(WCHAR))); dwSize <<= 1) {
		db_filename = (char*)p;
		DWORD dwLen = GetModuleFileNameW(NULL, p, dwSize);
		if (dwLen == 0) break;
		if (dwLen < dwSize) {
			int utf8size = WideCharToMultiByte(CP_UTF8, 0, p, -1, NULL, 0, NULL, NULL);
			if (utf8size) {
				if (char* q = (char*)malloc(utf8size + sizeof(DB_FILENAME))) {
					free(db_filename);
					db_filename = q;
					WideCharToMultiByte(CP_UTF8, 0, p, -1, db_filename, utf8size, NULL, NULL);
					strcpy(PathFindFileNameA(db_filename), DB_FILENAME);
					goto LABEL_RESOLVED_DBNAME;
				}
			}
		}
	}
#elif defined(__APPLE__)
	uint32_t size = PATH_MAX;
	for (char* p; (p = (char*)realloc(db_filename, size)); db_filename = p) {
		if (0 == _NSGetExecutablePath(p, &size)) {
			(db_filename = realpath(p, nullptr)) ? free(p) : (void)(db_filename = p);
			if ((p = strrchr(db_filename, '/'))) {
				if (ptrdiff_t o = (p - db_filename) + 1; (p = (char*) realloc(db_filename, o + sizeof(DB_FILENAME)))) {
					strcpy((db_filename = p) + o, DB_FILENAME);
					goto LABEL_RESOLVED_DBNAME;
				}
			}
		}
	}
#else
	/* TODO add more link options */
	size_t size = PATH_MAX;
	for (char* p; (p = (char*)realloc(db_filename, size)); size <<= 1) {
		ssize_t len = readlink("/proc/self/exe", db_filename = p, size);
		if (len < 0) break;
		if (len + sizeof(DB_FILENAME) < size) {
			if (!(p = strrchr(db_filename, '/'))) break;
			strcpy(p + 1, DB_FILENAME);
			goto LABEL_RESOLVED_DBNAME;
		}
	}
#endif
	free(db_filename);
	cUtils::PrintErr("Fatal initialisation error.");
	exit(EXIT_FAILURE);

LABEL_RESOLVED_DBNAME:
	/* Open a database handle */
	sqlite3* db;
	int rc = sqlite3_open_v2(db_filename, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE /*| SQLITE_OPEN_FULLMUTEX*/, nullptr);
	free(db_filename);
	/* If database handle is open, create leaderboard table */
	if (rc == SQLITE_OK) {
		sqlite3_stmt* stmt = nullptr;
		//const char* query = QUERY_INIT;
		//const char* end;
		for (const char *query = QUERY_INIT, *end; SQLITE_OK == sqlite3_prepare_v2(db, query, -1, &stmt, &end); query = end) {
			if (stmt) {
				int status = sqlite3_step(stmt);
				sqlite3_finalize(stmt);
				if (status != SQLITE_DONE)
					break;
			}
			if (!stmt) {
				g_db = db;
				cUtils::PrintLog("Database initialized successfully");
				return;
			}
		}
		// Error
		// ...
#if 0
		if (SQLITE_OK == sqlite3_prepare_v2(db, query, /*QRLEN_INIT*/sizeof(QUERY_INIT), &stmt, &end)) {
			if (SQLITE_DONE == sqlite3_step(stmt)) {
				sqlite3_finalize(stmt);
				g_db = db;

				if (SQLITE_OK == sqlite3_prepare_v2(db, end, sizeof(QUERY_INIT) - (end-query), &stmt, &end)) {
					if (SQLITE_DONE == sqlite3_step(stmt))
						cUtils::PrintLog("Ola ok 2");
					else
						cUtils::PrintLog("Ola skata, o nikic ftaiei 2 {:?}", end);
				}
				else
					cUtils::PrintLog("Ola skata, o nikic ftaiei {:?}", end);

				cUtils::PrintLog("Database initialized successfully {:?}.", end);
				return;
			}
		}
		sqlite3_finalize(stmt);
#endif
	}
	cUtils::PrintErr("Fatal Database Error: {}", sqlite3_errmsg(db));
	sqlite3_close(db);
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
	/* Add current task to the queue */
	std::lock_guard _(g_mutex);
	g_queue.emplace_back([this, h]() { m_func(h); });
	if (g_queue.size() == 1) {
		/* If there's only one task, allow thread to exit gracefully and restart */
		g_mutex.unlock();
		if (g_thread.joinable())
			g_thread.join();
		g_thread = std::thread([]() {
			for (;;) {
				g_queue.front()();
				std::lock_guard _(g_mutex);
				g_queue.pop_front();
				if (g_queue.empty())
					break;
			}
		});
	}
}

cDatabaseTask<>
cDatabase::UpdateLeaderboard(const cMessage& msg) {
	return [&msg]() {
		using namespace std::chrono;
		/* Execute QUERY_UPDATE_LB */
		sqlite3_stmt* stmt;
		if (SQLITE_OK == sqlite3_prepare_v2(g_db, QUERY_UPDATE_LB, QRLEN_UPDATE_LB, &stmt, nullptr)) {
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

cDatabaseTask<>
cDatabase::RegisterWelcome(const cUser& user) {
	return [&user]() {
		sqlite3_stmt* stmt;
		if (SQLITE_OK == sqlite3_prepare_v2(g_db, QUERY_REGISTER_WC, sizeof(QUERY_REGISTER_WC), &stmt, nullptr)) {
			if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, user.GetId().ToInt())) {
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

cDatabaseTask<bool>
cDatabase::WelcomeHasNullMessage(const cUser& user) {
	return [&user]() -> bool {
		sqlite3_stmt* stmt;
		if (SQLITE_OK == sqlite3_prepare_v2(g_db, QUERY_NULL_MSG, sizeof(QUERY_NULL_MSG), &stmt, nullptr)) {
			if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, user.GetId().ToInt())) {
				if (SQLITE_ROW == sqlite3_step(stmt)) {
					auto result = sqlite3_column_int64(stmt, 0);
					sqlite3_finalize(stmt);
					return result;
				}
			}
		}
		sqlite3_finalize(stmt);
		throw xDatabaseError();
	};
}

cDatabaseTask<cSnowflake>
cDatabase::WelcomeGetMessage(const cUser& user) {
	return [&user]() -> cSnowflake {
		sqlite3_stmt* stmt = nullptr;
		if (SQLITE_OK == sqlite3_prepare_v2(g_db, QUERY_WC_GET_MSG, sizeof(QUERY_WC_GET_MSG), &stmt, nullptr)) {
			if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, user.GetId().ToInt())) {
				switch (sqlite3_step(stmt)) {
					case SQLITE_DONE:
						sqlite3_finalize(stmt);
						return {};
					case SQLITE_ROW: {
						const int type = sqlite3_column_type(stmt, 0);
						const int64_t result = type == SQLITE_NULL ? 0 : sqlite3_column_int64(stmt, 0);
						sqlite3_finalize(stmt);
						return result;
					}
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
cDatabase::RegisterMessage(const cUser& user, const cMessage& msg) {
	return [&user, &msg]() {
		sqlite3_stmt* stmt = nullptr;
		if (SQLITE_OK == sqlite3_prepare_v2(g_db, QUERY_WC_REG_MSG, sizeof(QUERY_WC_REG_MSG), &stmt, nullptr)) {
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
		if (SQLITE_OK == sqlite3_prepare_v2(g_db, QUERY_GET_RANK, QRLEN_GET_RANK, &stmt, nullptr)) {
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
		if (SQLITE_OK == (rc = sqlite3_prepare_v2(g_db, QUERY_GET_TOP_10, QRLEN_GET_TOP_10, &stmt, nullptr))) {
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