#include "Database.h"
#include "Queries.h"
#include "Utils.h"
#include <filesystem>
#include <boost/asio/strand.hpp>
#include <boost/asio/system_executor.hpp>
#include <sqlite3.h>
#if   defined _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <Windows.h>
#elif defined __APPLE__
#  include <mach-o/dyld.h>
#endif
/* ========== The global sqlite connection handle =================================================================== */
static sqlite3* g_db = nullptr;
/* ========== The database instance which invokes the constructor that initializes the sqlite connection ============ */
cDatabase cDatabase::ms_instance;
/* ========== The database exception which uses the global sqlite connection's error message and error code ========= */
xDatabaseError::xDatabaseError() : std::runtime_error(sqlite3_errmsg(g_db)), m_code(sqlite3_extended_errcode(g_db)) {}
/* ========== A helper function which returns a fully qualified UTF-8 path for the database file ==================== */
static std::u8string get_db_filename() {
	namespace fs = std::filesystem;
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
				throw;
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
/* ========== The database instance constructor which initializes the global sqlite connection ====================== */
void cDatabase::Initialize() {
	sqlite3* db = nullptr; // The database connection handle
	std::string err_msg;   // The error message string
	try {
		/* Open a database connection; NOMUTEX since we make sure to use the connection in a strand */
		constexpr int DB_FLAGS = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX;
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
	std::terminate();
}
/* ========== The database instance destructor which simply closes the sqlite connection ============================ */
cDatabase::~cDatabase() {
	/* Close the database connection */
	sqlite3_close(g_db);
}
/* ========== An awaitable which resumes a coroutine to the sqlite connection strand ================================ */
namespace net = boost::asio;
class resume_on_db_strand {
private:
	static inline net::strand<net::system_executor> ms_strand;
public:
	bool await_ready() {
		return ms_strand.running_in_this_thread();
	}
	void await_suspend(std::coroutine_handle<> h) {
		net::post(ms_strand, [h]{ h.resume(); });
	}
	void await_resume() {}
};
/* ================================================================================================================== */
cTask<>
cDatabase::UpdateLeaderboard(const cMessage& msg) {
	co_await resume_on_db_strand();

	using namespace std::chrono;
	/* Execute QUERY_UPDATE_LB */
	sqlite3_stmt* stmt;
	if (SQLITE_OK == sqlite3_prepare_v2(g_db, QUERY_UPDATE_LB, sizeof(QUERY_UPDATE_LB), &stmt, nullptr)) {
		if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, msg.GetAuthor().GetId().ToInt())) {
			if (SQLITE_OK == sqlite3_bind_int64(stmt, 2, time_point_cast<seconds>(msg.GetId().GetTimestamp()).time_since_epoch().count())) {
				if (SQLITE_DONE == sqlite3_step(stmt)) {
					sqlite3_finalize(stmt);
					co_return;
				}
			}
		}
	}
	sqlite3_finalize(stmt);
	throw xDatabaseError();
}
cTask<uint64_t>
cDatabase::WC_RegisterMember(const cMember& member) {
	co_await resume_on_db_strand();

	sqlite3_stmt* stmt = nullptr;
	if (SQLITE_OK == sqlite3_prepare_v2(g_db, QUERY_WC_REG_MBR, sizeof(QUERY_WC_REG_MBR), &stmt, nullptr)) {
		if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, member.GetUser()->GetId().ToInt())) {
			if (SQLITE_OK == sqlite3_bind_int64(stmt, 2, member.JoinedAt().time_since_epoch().count())) {
				if (SQLITE_ROW == sqlite3_step(stmt)) {
					int64_t result = sqlite3_column_int64(stmt, 0);
					sqlite3_finalize(stmt);
					co_return static_cast<uint64_t>(result);
				}
			}
		}
	}
	sqlite3_finalize(stmt);
	throw xDatabaseError();
}
cTask<int64_t>
cDatabase::WC_GetMessage(const cMemberUpdate& member) {
	co_await resume_on_db_strand();

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
					co_return result;
				default:
					break;
			}
		}
	}
	sqlite3_finalize(stmt);
	throw xDatabaseError();
}
cTask<>
cDatabase::WC_EditMessage(int64_t msg_id) {
	co_await resume_on_db_strand();

	sqlite3_stmt* stmt = nullptr;
	if (SQLITE_OK == sqlite3_prepare_v2(g_db, QUERY_WC_EDT_MSG, sizeof(QUERY_WC_EDT_MSG), &stmt, nullptr)) {
		if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, msg_id)) {
			if (SQLITE_DONE == sqlite3_step(stmt)) {
				sqlite3_finalize(stmt);
				co_return;
			}
		}
	}
	sqlite3_finalize(stmt);
	throw xDatabaseError();
}
cTask<uint64_t>
cDatabase::WC_DeleteMember(const cUser& user) {
	co_await resume_on_db_strand();

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
					co_return result;
			}
		}
	}
	sqlite3_finalize(stmt);
	throw xDatabaseError();
}
cTask<>
cDatabase::WC_UpdateMessage(const cUser& user, const cMessage& msg) {
	co_await resume_on_db_strand();

	sqlite3_stmt* stmt = nullptr;
	if (SQLITE_OK == sqlite3_prepare_v2(g_db, QUERY_WC_UPD_MSG, sizeof(QUERY_WC_UPD_MSG), &stmt, nullptr)) {
		if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, user.GetId().ToInt())) {
			if (SQLITE_OK == sqlite3_bind_int64(stmt, 2, msg.GetId().ToInt())) {
				if (SQLITE_DONE == sqlite3_step(stmt)) {
					sqlite3_finalize(stmt);
					co_return;
				}
			}
		}
	}
	sqlite3_finalize(stmt);
	throw xDatabaseError();
}
cTask<int64_t>
cDatabase::SB_GetMessageAuthor(const cSnowflake& msg_id) {
	co_await resume_on_db_strand();

	sqlite3_stmt* stmt = nullptr;
	if (SQLITE_OK == sqlite3_prepare_v2(g_db, QUERY_SB_GET_MESSAGE_AUTHOR, sizeof(QUERY_SB_GET_MESSAGE_AUTHOR), &stmt, nullptr)) {
		if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, msg_id.ToInt())) {
			switch(int64_t result = 0; sqlite3_step(stmt)) {
				case SQLITE_ROW:
					result = sqlite3_column_int64(stmt, 0);
				case SQLITE_DONE:
					sqlite3_finalize(stmt);
					co_return result;
				default:
					break;
			}
		}
	}
	sqlite3_finalize(stmt);
	throw xDatabaseError();
}
cTask<std::pair<int64_t, int64_t>>
cDatabase::SB_RegisterReaction(const cSnowflake& msg_id, const cSnowflake& author_id) {
	co_await resume_on_db_strand();

	sqlite3_stmt* stmt = nullptr;
	if (SQLITE_OK == sqlite3_prepare_v2(g_db, QUERY_SB_REGISTER_REACTION, sizeof(QUERY_SB_REGISTER_REACTION), &stmt, nullptr)) {
		if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, msg_id.ToInt())) {
			if (SQLITE_OK == sqlite3_bind_int64(stmt, 2, author_id.ToInt())) {
				if (SQLITE_ROW == sqlite3_step(stmt)) {
					std::pair<int64_t, int64_t> result{sqlite3_column_int64(stmt, 0), sqlite3_column_int64(stmt, 1)};
					sqlite3_finalize(stmt);
					co_return result;
				}
			}
		}
	}
	sqlite3_finalize(stmt);
	throw xDatabaseError();
}
cTask<>
cDatabase::SB_RegisterMessage(const cSnowflake& msg_id, const cSnowflake& sb_msg_id) {
	co_await resume_on_db_strand();

	sqlite3_stmt* stmt = nullptr;
	if (SQLITE_OK == sqlite3_prepare_v2(g_db, QUERY_SB_REGISTER_MESSAGE, sizeof(QUERY_SB_REGISTER_MESSAGE), &stmt, nullptr)) {
		if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, msg_id.ToInt())) {
			if (SQLITE_OK == sqlite3_bind_int64(stmt, 2, sb_msg_id.ToInt())) {
				if (SQLITE_DONE == sqlite3_step(stmt)) {
					sqlite3_finalize(stmt);
					co_return;
				}
			}
		}
	}
	sqlite3_finalize(stmt);
	throw xDatabaseError();
}
cTask<std::pair<int64_t, int64_t>>
cDatabase::SB_RemoveReaction(const cSnowflake& msg_id) {
	co_await resume_on_db_strand();

	std::pair<int64_t, int64_t> result;

	sqlite3_stmt* stmt = nullptr;
	for (const char* query = QUERY_SB_REMOVE_REACTION, *end; SQLITE_OK == sqlite3_prepare_v2(g_db, query, -1, &stmt, &end); query = end) {
		if (!stmt)
			co_return result;
		if (SQLITE_OK != sqlite3_bind_int64(stmt, 1, msg_id.ToInt()))
			break;
		switch (sqlite3_step(stmt)) {
			case SQLITE_ROW:
				result = {sqlite3_column_int64(stmt, 0), sqlite3_column_int64(stmt, 1)};
			case SQLITE_OK:
				sqlite3_finalize(stmt);
				co_return result;
			default:
				break;
		}
	}
	sqlite3_finalize(stmt);
	throw xDatabaseError();
}
cTask<>
cDatabase::SB_RemoveMessage(const cSnowflake& msg_id) {
	co_await resume_on_db_strand();

	sqlite3_stmt* stmt = nullptr;
	if (SQLITE_OK == sqlite3_prepare_v2(g_db, QUERY_SB_REMOVE_MESSAGE, sizeof(QUERY_SB_REMOVE_MESSAGE), &stmt, nullptr)) {
		if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, msg_id.ToInt())) {
			if (SQLITE_DONE == sqlite3_step(stmt)) {
				sqlite3_finalize(stmt);
				co_return;
			}
		}
	}
	sqlite3_finalize(stmt);
	throw xDatabaseError();
}
cTask<int64_t>
cDatabase::SB_RemoveAll(const cSnowflake& msg_id) {
	co_await resume_on_db_strand();

	sqlite3_stmt* stmt = nullptr;
	if (SQLITE_OK == sqlite3_prepare_v2(g_db, QUERY_SB_REMOVE_ALL, sizeof(QUERY_SB_REMOVE_ALL), &stmt, nullptr)) {
		if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, msg_id.ToInt())) {
			switch (int64_t result = 0; sqlite3_step(stmt)) {
				case SQLITE_ROW:
					result = sqlite3_column_int64(stmt, 0);
				case SQLITE_DONE:
					sqlite3_finalize(stmt);
					co_return result;
				default:
					break;
			}
		}
	}
	sqlite3_finalize(stmt);
	throw xDatabaseError();
}
cTask<std::vector<starboard_entry>>
cDatabase::SB_GetTop10(int threshold) {
	co_await resume_on_db_strand();

	std::vector<starboard_entry> result;
	sqlite3_stmt* stmt = nullptr;
	int res = SQLITE_OK;
	if (SQLITE_OK == sqlite3_prepare_v2(g_db, QUERY_SB_GET_TOP_10, sizeof(QUERY_SB_GET_TOP_10), &stmt, nullptr)) {
		if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, threshold)) {
			result.reserve(10);
			for (int64_t rank = 1; SQLITE_ROW == (res = sqlite3_step(stmt)); ++rank) {
				result.emplace_back(
					sqlite3_column_int64(stmt, 0),
					sqlite3_column_int64(stmt, 1),
					sqlite3_column_int64(stmt, 2),
					sqlite3_column_int64(stmt, 3),
					rank
				);
			}
		}
	}
	sqlite3_finalize(stmt);
	if (res == SQLITE_DONE)
		co_return result;
	throw xDatabaseError();
}
cTask<std::vector<starboard_entry>>
cDatabase::SB_GetRank(const cUser& user, int threshold) {
	co_await resume_on_db_strand();
	sqlite3_stmt* stmt = nullptr;
	if (SQLITE_OK == sqlite3_prepare_v2(g_db, QUERY_SB_GET_RANK, sizeof(QUERY_SB_GET_RANK), &stmt, nullptr)) {
		if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, user.GetId().ToInt())) {
			if (SQLITE_OK == sqlite3_bind_int64(stmt, 2, threshold)) {
				std::vector<starboard_entry> result;
				switch (sqlite3_step(stmt)) {
					case SQLITE_ROW:
						result.emplace_back(
							sqlite3_column_int64(stmt, 0),
							sqlite3_column_int64(stmt, 1),
							sqlite3_column_int64(stmt, 2),
							sqlite3_column_int64(stmt, 3),
							sqlite3_column_int64(stmt, 4)
						);
					case SQLITE_DONE:
						sqlite3_finalize(stmt);
						co_return result;
					default:
						break;
				}
			}
		}
	}
	sqlite3_finalize(stmt);
	throw xDatabaseError();
}
cTask<tRankQueryData>
cDatabase::GetUserRank(const cUser& user) {
	co_await resume_on_db_strand();
	sqlite3_stmt* stmt = nullptr;
	if (SQLITE_OK == sqlite3_prepare_v2(g_db, QUERY_GET_RANK, sizeof(QUERY_GET_RANK), &stmt, nullptr)) {
		if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, user.GetId().ToInt())) {
			tRankQueryData result;
			switch (sqlite3_step(stmt)) {
				case SQLITE_ROW:
					/* Statement returned data for the user */
					result.emplace_back(
						sqlite3_column_int64(stmt, 0),
						user.GetId(),
						sqlite3_column_int64(stmt, 1),
						sqlite3_column_int64(stmt, 2)
					);
				case SQLITE_DONE:
					/* Statement didn't return, no user data found */
					sqlite3_finalize(stmt);
					co_return result;
				default:
					break;
			}
		}
	}
	sqlite3_finalize(stmt);
	throw xDatabaseError();
}
cTask<tRankQueryData>
cDatabase::GetTop10() {
	co_await resume_on_db_strand();
	int rc = SQLITE_OK;
	tRankQueryData res;
	sqlite3_stmt* stmt = nullptr;
	if (SQLITE_OK == sqlite3_prepare_v2(g_db, QUERY_GET_TOP_10, sizeof(QUERY_GET_TOP_10), &stmt, nullptr)) {
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
		co_return res;
	throw xDatabaseError();
}
cTask<>
cDatabase::RegisterMessage(const cMessage& msg) {
	co_await resume_on_db_strand();
	sqlite3_stmt* stmt = nullptr;
	if (SQLITE_OK == sqlite3_prepare_v2(g_db, QUERY_REGISTER_MESSAGE, sizeof QUERY_REGISTER_MESSAGE, &stmt, nullptr)) {
		if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, msg.GetId().ToInt())) {
			if (SQLITE_OK == sqlite3_bind_int64(stmt, 2, msg.GetChannelId().ToInt())) {
				if (SQLITE_OK == sqlite3_bind_int64(stmt, 3, msg.GetAuthor().GetId().ToInt())) {
					auto content = msg.GetContent();
					if (SQLITE_OK == (content.empty() ? sqlite3_bind_null(stmt, 4) : sqlite3_bind_text64(stmt, 4, content.data(), content.size(), SQLITE_STATIC, SQLITE_UTF8))) {
						if (SQLITE_DONE == sqlite3_step(stmt)) {
							sqlite3_finalize(stmt);
							co_return;
						}
					}
				}
			}
		}
	}
	sqlite3_finalize(stmt);
	throw xDatabaseError();
}
cTask<message_entry>
cDatabase::GetMessage(const cSnowflake& id) {
	co_await resume_on_db_strand();
	sqlite3_stmt* stmt = nullptr;
	std::unique_ptr<sqlite3_stmt, int(*)(sqlite3_stmt*)> unique_stmt(stmt, sqlite3_finalize);
	if (SQLITE_OK == sqlite3_prepare_v2(g_db, QUERY_GET_MESSAGE, sizeof QUERY_GET_MESSAGE, &stmt, nullptr)) {
		unique_stmt.reset(stmt);
		if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, id.ToInt())) {
			if (SQLITE_ROW == sqlite3_step(stmt)) {
				message_entry msg {
					id,
					sqlite3_column_int64(stmt, 0),
					sqlite3_column_int64(stmt, 1)
				};
				if (auto text = sqlite3_column_text(stmt, 2))
					msg.content = (const char*)text;
				co_return msg;
			}
		}
	}
	throw xDatabaseError();
}
cTask<std::optional<message_entry>>
cDatabase::DeleteMessage(const cSnowflake& id) {
	co_await resume_on_db_strand();
	sqlite3_stmt* stmt = nullptr;
	std::unique_ptr<sqlite3_stmt, int(*)(sqlite3_stmt*)> unique_stmt(nullptr, sqlite3_finalize);
	if (SQLITE_OK == sqlite3_prepare_v2(g_db, QUERY_DELETE_MESSAGE, sizeof QUERY_DELETE_MESSAGE, &stmt, nullptr)) {
		unique_stmt.reset(stmt);
		if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, id.ToInt())) {
			std::optional<message_entry> result;
			switch (sqlite3_step(stmt)) {
				case SQLITE_ROW:
					result.emplace(
						id,
						sqlite3_column_int64(stmt, 0),
						sqlite3_column_int64(stmt, 1)
					);
					if (auto text = sqlite3_column_text(stmt, 2))
						result->content = (const char*)text;
				case SQLITE_DONE:
					co_return result;
			}
		}
	}
	throw xDatabaseError();
}
cTask<std::vector<message_entry>>
cDatabase::DeleteMessages(std::span<const cSnowflake> ids) {
	co_await resume_on_db_strand();
	if (ids.empty())
		co_return {};

	/* Make query */
	std::string query = "DELETE FROM messages WHERE id IN (";
	for (int i = 0; i < ids.size(); ++i)
		query += "?,";
	query.back() = ')';
	query += " RETURNING *;";
	/* Prepare statement */
	sqlite3_stmt* stmt = nullptr;
	if (SQLITE_OK != sqlite3_prepare_v2(g_db, query.c_str(), query.size(), &stmt, nullptr))
		throw xDatabaseError();
	std::unique_ptr<sqlite3_stmt, int(*)(sqlite3_stmt*)> unique_stmt(stmt, sqlite3_finalize);
	/* Bind ids to the statement */
	for (int i = 1; i <= ids.size(); ++i) {
		if (SQLITE_OK != sqlite3_bind_int64(stmt, i, ids[i].ToInt()))
			throw xDatabaseError();
	}
	std::vector<message_entry> result;
	result.reserve(ids.size());
	for (;;) {
		switch (sqlite3_step(stmt)) {
			case SQLITE_DONE:
				co_return result;
			case SQLITE_ROW: {
				auto& entry = result.emplace_back(
					sqlite3_column_int64(stmt, 0),
					sqlite3_column_int64(stmt, 1),
					sqlite3_column_int64(stmt, 2)
				);
				if (auto text = sqlite3_column_text(stmt, 3))
					entry.content = (const char*)text;
				break;
			}
			default:
				throw xDatabaseError();
		}
	}
}
cTask<>
cDatabase::CleanupMessages() {
	using namespace std::chrono;
	using namespace std::chrono_literals;
	co_await resume_on_db_strand();
	sqlite3_stmt* stmt = nullptr;
	if (SQLITE_OK == sqlite3_prepare_v2(g_db, QUERY_CLEANUP_MESSAGES, sizeof QUERY_CLEANUP_MESSAGES, &stmt, nullptr)) {
		if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, duration_cast<milliseconds>(system_clock::now() - sys_days(2015y/1/1) - 15 * 24h).count())) {
			if (SQLITE_DONE == sqlite3_step(stmt)) {
				sqlite3_finalize(stmt);
				co_return;
			}
		}
	}
	sqlite3_finalize(stmt);
	throw xDatabaseError();
}