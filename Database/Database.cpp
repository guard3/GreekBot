#include "Database.h"
#include "Queries.h"
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

xDatabaseError::xDatabaseError(sqlite3* db) : std::runtime_error(sqlite3_errmsg(db)), m_code(sqlite3_extended_errcode(db)) {}

cDatabase cDatabase::ms_instance;
sqlite3* cDatabase::ms_pDB;

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
	int rc = sqlite3_open_v2(db_filename, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, nullptr);
	free(db_filename);
	/* If database handle is open, create leaderboard table */
	if (rc == SQLITE_OK) {
		sqlite3_stmt* stmt;
		if (SQLITE_OK == sqlite3_prepare_v2(db, QUERY_INIT, QRLEN_INIT, &stmt, nullptr)) {
			if (SQLITE_DONE == sqlite3_step(stmt)) {
				sqlite3_finalize(stmt);
				ms_pDB = db;
				cUtils::PrintLog("Database initialized successfully.");
				return;
			}
		}
		sqlite3_finalize(stmt);
	}
	cUtils::PrintErr("Fatal Database Error: %s", sqlite3_errmsg(db));
	sqlite3_close(db);
	exit(EXIT_FAILURE);
}

cDatabase::~cDatabase() { sqlite3_close(ms_pDB); }

class cTaskManager final {
private:
	static cTaskManager ms_instance;
	static inline std::deque<std::function<void()>> ms_queue;
	static inline std::thread ms_thread;
	static inline std::mutex ms_mutex;

	static void thread_func() {
		for (;;) {
			ms_queue.front()();
			ms_mutex.lock();
			ms_queue.pop_front();
			if (ms_queue.empty())
				break;
			ms_mutex.unlock();
		}
		ms_mutex.unlock();
	}

	cTaskManager() = default;

public:
	cTaskManager(const cTaskManager&) = delete;
	cTaskManager(cTaskManager&&) = delete;
	~cTaskManager() {
		if (ms_thread.joinable())
			ms_thread.join();
	}

	static void CreateTask(std::function<void()> func) {
		ms_mutex.lock();
		ms_queue.push_back(func);
		if (ms_queue.size() == 1) {
			ms_mutex.unlock();
			if (ms_thread.joinable())
				ms_thread.join();
			ms_thread = std::thread(thread_func);
			return;
		}
		ms_mutex.unlock();
	}
} cTaskManager::ms_instance;

cTask<>
cDatabase::UpdateLeaderboard(const cMessage& msg) {
	class awaitable {
	private:
		const cMessage& m_msg;
		std::exception_ptr m_except;

	public:
		awaitable(const cMessage& msg) : m_msg(msg) {}

		bool await_ready() { return false; }
		void await_suspend(std::coroutine_handle<> h) {
			cTaskManager::CreateTask([this, h]() {
				/* Execute QUERY_UPDATE_LB */
				sqlite3_stmt* stmt;
				if (SQLITE_OK == sqlite3_prepare_v2(ms_pDB, QUERY_UPDATE_LB, QRLEN_UPDATE_LB, &stmt, nullptr)) {
					if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, m_msg.GetAuthor().GetId().ToInt())) {
						if (SQLITE_OK == sqlite3_bind_int64(stmt, 2, m_msg.GetId().GetTimestamp())) {
							if (SQLITE_DONE == sqlite3_step(stmt))
								goto LABEL_DONE;
						}
					}
				}
				m_except = std::make_exception_ptr(xDatabaseError(ms_pDB));
			LABEL_DONE:
				sqlite3_finalize(stmt);
				h();
			});
		}
		void await_resume() {
			if (m_except)
				std::rethrow_exception(m_except);
		}
	};

	co_await awaitable(msg);
}

cTask<tRankQueryData>
cDatabase::GetUserRank(const cUser& user) {
	class awaitable {
	private:
		const cUser& m_user;
		std::exception_ptr m_except;
		tRankQueryData m_result;

	public:
		awaitable(const cUser& u) : m_user(u) {}

		bool await_ready() { return false; }
		void await_suspend(std::coroutine_handle<> h) {
			cTaskManager::CreateTask([this, h]() {
				sqlite3_stmt* stmt;
				if (SQLITE_OK == sqlite3_prepare_v2(ms_pDB, QUERY_GET_RANK, QRLEN_GET_RANK, &stmt, nullptr)) {
					if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, m_user.GetId().ToInt())) {
						switch (sqlite3_step(stmt)) {
							case SQLITE_ROW:
								/* Statement returned data for the user */
								m_result.emplace_back(
									sqlite3_column_int64(stmt, 0),
									m_user.GetId(),
									sqlite3_column_int64(stmt, 1),
									sqlite3_column_int64(stmt, 2)
								);
							case SQLITE_DONE:
								/* Statement didn't return, no user data found */
								goto LABEL_DONE;
						}
					}
				}
				m_except = std::make_exception_ptr(xDatabaseError(ms_pDB));
			LABEL_DONE:
				sqlite3_finalize(stmt);
				h();
			});
		}
		tRankQueryData await_resume() {
			if (m_except)
				std::rethrow_exception(m_except);
			return std::move(m_result);
		}
	};
	co_return co_await awaitable(user);
}

cTask<tRankQueryData>
cDatabase::GetTop10() {
	class awaitable {
	private:
		std::exception_ptr m_except;
		tRankQueryData m_result;

	public:
		bool await_ready() { return false; }
		void await_suspend(std::coroutine_handle<> h) {
			cTaskManager::CreateTask([this, h]() {
				int rc;
				sqlite3_stmt* stmt;
				if (SQLITE_OK == (rc = sqlite3_prepare_v2(ms_pDB, QUERY_GET_TOP_10, QRLEN_GET_TOP_10, &stmt, nullptr))) {
					m_result.reserve(10);
					while (SQLITE_ROW == (rc = sqlite3_step(stmt))) {
						m_result.emplace_back(
							sqlite3_column_int64(stmt, 0),
							sqlite3_column_int64(stmt, 1),
							sqlite3_column_int64(stmt, 2),
							sqlite3_column_int64(stmt, 3)
						);
					}
				}
				sqlite3_finalize(stmt);
				if (rc != SQLITE_DONE)
					m_except = std::make_exception_ptr(xDatabaseError(ms_pDB));
				h();
			});
		}
		tRankQueryData await_resume() {
			if (m_except)
				std::rethrow_exception(m_except);
			return std::move(m_result);
		}
	};
	co_return co_await awaitable();
}