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
#if defined _WIN32
	std::vector<WCHAR> szPath(MAX_PATH);
	for (DWORD dwLen;;) {
		/* Retrieve the fully qualified path of the executable */
		dwLen = GetModuleFileNameW(NULL, szPath.data(), (DWORD)szPath.size());
		/* If the length is 0, an error occured */
		if (dwLen == 0) {
			const DWORD dwMsg = GetLastError();
			LPSTR lpszMsg = NULL;
			try {
				constexpr DWORD FORMAT_FLAGS = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
				if (0 == FormatMessageA(FORMAT_FLAGS, NULL, dwMsg, 0, (LPSTR)&lpszMsg, 256, NULL))
					throw std::runtime_error("Cannot find a suitable filepath.");
				throw std::runtime_error(lpszMsg);
			} catch (...) {
				LocalFree(lpszMsg);
				std::rethrow_exception(std::current_exception());
			}
		}
		/* If the length is smaller than the total vector size, then the full path has been retrieved */
		if (dwLen < szPath.size()) {
			szPath.resize(dwLen);
			break;
		}
		/* Otherwise, increase the vector size and retry */
		szPath.resize(dwLen * 2);
	}
	fs::path result{ szPath.begin(), szPath.end() };
#elif defined __APPLE__
	std::vector<char> path(256);
	/* Retrieve 'a' path to the executable. */
	if (uint32_t size = 256; _NSGetExecutablePath(path.data(), &size) < 0) {
		/* If the path vector isn't large enough, 'size' is updated to the required size */
		path.resize(size);
		/* Retry */
		_NSGetExecutablePath(path.data(), &size);
	}
	fs::path result = fs::canonical(path.data());
#else
	fs::path result = fs::canonical("/proc/self/exe");
#endif
	/* Replace the filename and convert the final path to a UTF-8 encoded string */
	return result.replace_filename("database.db").u8string();
}

cDatabase::cDatabase() {
	sqlite3* db = nullptr; // The database connection handle
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
		err_msg = "An exception was thrown.";
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

cDatabaseTask<int64_t>
cDatabase::SB_GetMessageAuthor(const cSnowflake& msg_id) {
	return [&msg_id] {
		sqlite3_stmt* stmt = nullptr;
		if (SQLITE_OK == sqlite3_prepare_v2(g_db, QUERY_SB_GET_MESSAGE_AUTHOR, sizeof(QUERY_SB_GET_MESSAGE_AUTHOR), &stmt, nullptr)) {
			if (stmt) {
				if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, msg_id.ToInt())) {
					switch(int64_t result = 0; sqlite3_step(stmt)) {
						case SQLITE_ROW:
							result = sqlite3_column_int64(stmt, 0);
						case SQLITE_DONE:
							sqlite3_finalize(stmt);
							return result;
						default:
							break;
					}
				}
			}
		}
		sqlite3_finalize(stmt);
		throw xDatabaseError();
	};
}

cDatabaseTask<std::pair<int64_t, int64_t>>
cDatabase::SB_RegisterReaction(const cSnowflake& msg_id, const cSnowflake& author_id) {
	return [&msg_id, &author_id] {
		sqlite3_stmt* stmt = nullptr;
		if (SQLITE_OK == sqlite3_prepare_v2(g_db, QUERY_SB_REGISTER_REACTION, sizeof(QUERY_SB_REGISTER_REACTION), &stmt, nullptr)) {
			if (stmt) {
				if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, msg_id.ToInt())) {
					if (SQLITE_OK == sqlite3_bind_int64(stmt, 2, author_id.ToInt())) {
						if (SQLITE_ROW == sqlite3_step(stmt)) {
							std::pair<int64_t, int64_t> result{sqlite3_column_int64(stmt, 0), sqlite3_column_int64(stmt, 1)};
							sqlite3_finalize(stmt);
							return result;
						}
					}
				}
			}
		}
		sqlite3_finalize(stmt);
		throw xDatabaseError();
	};
}

cDatabaseTask<>
cDatabase::SB_RegisterMessage(const cSnowflake& msg_id, const cSnowflake& sb_msg_id) {
	return [&msg_id, &sb_msg_id] {
		sqlite3_stmt* stmt = nullptr;
		if (SQLITE_OK == sqlite3_prepare_v2(g_db, QUERY_SB_REGISTER_MESSAGE, sizeof(QUERY_SB_REGISTER_MESSAGE), &stmt, nullptr)) {
			if (stmt) {
				if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, msg_id.ToInt())) {
					if (SQLITE_OK == sqlite3_bind_int64(stmt, 2, sb_msg_id.ToInt())) {
						if (SQLITE_DONE == sqlite3_step(stmt)) {
							sqlite3_finalize(stmt);
							return;
						}
					}
				}
			}
		}
		sqlite3_finalize(stmt);
		throw xDatabaseError();
	};
}

cDatabaseTask<std::pair<int64_t, int64_t>>
cDatabase::SB_RemoveReaction(const cSnowflake& msg_id) {
	return [&msg_id] {
		std::pair<int64_t, int64_t> result;

		sqlite3_stmt* stmt = nullptr;
		for (const char* query = QUERY_SB_REMOVE_REACTION, *end; SQLITE_OK == sqlite3_prepare_v2(g_db, query, -1, &stmt, &end); query = end) {
			if (!stmt)
				return result;
			if (SQLITE_OK != sqlite3_bind_int64(stmt, 1, msg_id.ToInt()))
				break;
			switch (sqlite3_step(stmt)) {
				case SQLITE_ROW:
					result = {sqlite3_column_int64(stmt, 0), sqlite3_column_int64(stmt, 1)};
				case SQLITE_OK:
					sqlite3_finalize(stmt);
					return result;
				default:
					break;
			}
		}
#if 0
		if (SQLITE_OK == sqlite3_prepare_v2(g_db, QUERY_SB_REMOVE_REACTION, sizeof(QUERY_SB_REMOVE_REACTION), &stmt, nullptr)) {
			if (stmt) {
				if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, msg_id.ToInt())) {
					if (SQLITE_ROW == sqlite3_step(stmt)) {
						std::pair<cSnowflake, uint64_t> result{ sqlite3_column_int64(stmt, 0), sqlite3_column_int64(stmt, 1) };
						sqlite3_finalize(stmt);
						return result;
					}
				}
			}
		}
#endif
		sqlite3_finalize(stmt);
		throw xDatabaseError();
	};
}

cDatabaseTask<>
cDatabase::SB_RemoveMessage(const cSnowflake& msg_id) {
	return [&msg_id] {
		sqlite3_stmt* stmt = nullptr;
		if (SQLITE_OK == sqlite3_prepare_v2(g_db, QUERY_SB_REMOVE_MESSAGE, sizeof(QUERY_SB_REMOVE_MESSAGE), &stmt, nullptr)) {
			if (stmt) {
				if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, msg_id.ToInt())) {
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

cDatabaseTask<int64_t>
cDatabase::SB_RemoveAll(const cSnowflake& msg_id) {
	return [&msg_id] {
		sqlite3_stmt* stmt = nullptr;
		if (SQLITE_OK == sqlite3_prepare_v2(g_db, QUERY_SB_REMOVE_ALL, sizeof(QUERY_SB_REMOVE_ALL), &stmt, nullptr)) {
			if (stmt) {
				if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, msg_id.ToInt())) {
					switch (int64_t result = 0; sqlite3_step(stmt)) {
						case SQLITE_ROW:
							result = sqlite3_column_int64(stmt, 0);
						case SQLITE_DONE:
							sqlite3_finalize(stmt);
							return result;
						default:
							break;
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