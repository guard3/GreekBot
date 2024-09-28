#include "Database.h"
#include "Queries.h"
#include "Utils.h"
#include <thread>
#include <boost/asio/defer.hpp>
#include <boost/asio/io_context.hpp>
#include <sqlite3.h>
/* ========== The global sqlite connection handle =================================================================== */
static sqlite3* g_db = nullptr;
/* ========== The database instance which invokes the constructor that initializes the sqlite connection ============ */
cDatabase cDatabase::ms_instance;
/* ========== The database exception which uses the global sqlite connection's error message and error code ========= */
xDatabaseError::xDatabaseError() : std::runtime_error(sqlite3_errmsg(g_db)), m_code(sqlite3_extended_errcode(g_db)) {}
/* ========== A non owning view to an sqlite3 query string ========================================================== */
struct sqlite3_query_str {
	const char* str;
	int size;
	template<std::size_t N>
	consteval sqlite3_query_str(const char(&s)[N]) noexcept : str(s), size{} {
		std::size_t str_size = N + 1;
		size = str_size > std::numeric_limits<int>::max() ? - 1 : (int)str_size;
	}
	constexpr sqlite3_query_str(const char* s, std::size_t n) noexcept : str(s), size(n > std::numeric_limits<int>::max() ? -1 : (int)n) {}
	constexpr sqlite3_query_str(const std::string& s) noexcept : str(s.data()), size{} {
		std::size_t str_size = s.size() + 1;
		size = str_size > std::numeric_limits<int>::max() ? -1 : (int)str_size;
	}
};
/* ========== A smart owning pointer to a prepared sqlite3 statement, implicitly convertible to sqlite3_stmt* ======= */
class sqlite3_stmt_ptr final {
private:
	sqlite3_stmt* m_stmt;
public:
	/* Constructors */
	sqlite3_stmt_ptr() noexcept : m_stmt{} {}
	explicit sqlite3_stmt_ptr(sqlite3_stmt* pStmt) noexcept : m_stmt(pStmt) {}
	sqlite3_stmt_ptr(const sqlite3_stmt_ptr&) = delete;
	sqlite3_stmt_ptr(sqlite3_stmt_ptr&& o) noexcept : m_stmt(o.m_stmt) { o.m_stmt = nullptr; }
	/* Destructor */
	~sqlite3_stmt_ptr() { sqlite3_finalize(m_stmt); }
	/* Assignments */
	sqlite3_stmt_ptr& operator=(const sqlite3_stmt_ptr&) = delete;
	sqlite3_stmt_ptr& operator=(sqlite3_stmt_ptr&& o) noexcept {
		sqlite3_finalize(m_stmt);
		m_stmt = o.m_stmt;
		o.m_stmt = nullptr;
		return *this;
	}
	/* Reset owned pointer */
	void reset(sqlite3_stmt* stmt = nullptr) noexcept {
		sqlite3_finalize(m_stmt);
		m_stmt = stmt;
	}
	/* Operators */
	operator bool () noexcept { return m_stmt; }
	operator sqlite3_stmt* () noexcept { return m_stmt; }
};
/* ========== Custom function that returns a smart pointer to a prepared statement ================================== */
[[nodiscard]]
static sqlite3_stmt_ptr
sqlite3_prepare(sqlite3_query_str query) noexcept {
	sqlite3_stmt* stmt{};
	sqlite3_prepare_v2(g_db, query.str, query.size, &stmt, nullptr);
	return sqlite3_stmt_ptr(stmt);
}
/* ========== The database instance constructor which initializes the global sqlite connection ====================== */
void cDatabase::Initialize() {
	sqlite3* db = nullptr; // The database connection handle
	std::string err_msg;   // The error message string
	try {
		/* Open a database connection; NOMUTEX since we make sure to use the connection in a strand */
		constexpr int DB_FLAGS = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX;
		int rc = sqlite3_open_v2(reinterpret_cast<const char*>(cUtils::GetExecutablePath().replace_filename("database.db").u8string().c_str()), &db, DB_FLAGS, nullptr);
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
					cUtils::PrintDbg("Database initialized successfully");
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
	/* A custom io_context that runs on one strand and never runs out of work */
	static inline auto ms_ioc = [] {
		class _ : public net::io_context {
			net::executor_work_guard<net::io_context::executor_type> m_work_guard;
			std::thread m_work_thread;
		public:
			_() : net::io_context(1), m_work_guard(net::make_work_guard(*this)), m_work_thread([this] { run(); }) {}
			~_() {
				m_work_guard.reset();
				m_work_thread.join();
			}
		};
		return _();
	}();
public:
	bool await_ready() const {
		return ms_ioc.get_executor().running_in_this_thread();
	}
	void await_suspend(std::coroutine_handle<> h) const {
		net::defer(ms_ioc, [h]{ h.resume(); });
	}
	void await_resume() const {}
};
/* ================================================================================================================== */
cTask<std::uint64_t>
cDatabase::UpdateLeaderboard(const cMessage& msg) {
	using namespace std::chrono;
	co_await resume_on_db_strand();
	auto stmt = sqlite3_prepare(QUERY_UPDATE_LB);
	if (   stmt
	    && SQLITE_OK  == sqlite3_bind_int64(stmt, 1, msg.GetAuthor().GetId().ToInt())
	    && SQLITE_OK  == sqlite3_bind_int64(stmt, 2, floor<seconds>(msg.GetTimestamp()).time_since_epoch().count())) {
		switch (sqlite3_step(stmt)) {
			case SQLITE_DONE:
				co_return 0;
			case SQLITE_ROW:
				co_return sqlite3_column_int64(stmt, 0);
			default:
				break;
		}
	}
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
cTask<std::optional<leaderboard_entry>>
cDatabase::GetUserRank(const cUser& user) {
	co_await resume_on_db_strand();
	auto stmt = sqlite3_prepare(QUERY_GET_RANK);
	if (stmt && SQLITE_OK == sqlite3_bind_int64(stmt, 1, user.GetId().ToInt())) {
		std::optional<leaderboard_entry> result;
		switch (sqlite3_step(stmt)) {
			case SQLITE_ROW:
				result.emplace(
					user.GetId(),
					sqlite3_column_int64(stmt, 0),
					sqlite3_column_int64(stmt, 1),
					sqlite3_column_int64(stmt, 2)
				);
			case SQLITE_DONE:
				co_return result;
			default:
				break;
		}
	}
	throw xDatabaseError();
}
cTask<std::vector<leaderboard_entry>>
cDatabase::GetTop10() {
	co_await resume_on_db_strand();
	if (auto stmt = sqlite3_prepare(QUERY_GET_TOP_10)) {
		int rc;
		std::vector<leaderboard_entry> result;
		for (result.reserve(10); SQLITE_ROW == (rc = sqlite3_step(stmt));) {
			result.emplace_back(
				sqlite3_column_int64(stmt, 0),
				sqlite3_column_int64(stmt, 1),
				sqlite3_column_int64(stmt, 2),
				sqlite3_column_int64(stmt, 3)
			);
		}
		if (rc == SQLITE_DONE)
			co_return result;
	}
	throw xDatabaseError();
}
cTask<>
cDatabase::RegisterMessage(const cMessage& msg) {
	co_await resume_on_db_strand();
	auto stmt = sqlite3_prepare(QUERY_REGISTER_MESSAGE);
	if (   stmt
	    && SQLITE_OK == sqlite3_bind_int64(stmt, 1, msg.GetId().ToInt())
	    && SQLITE_OK == sqlite3_bind_int64(stmt, 2, msg.GetChannelId().ToInt())
	    && SQLITE_OK == sqlite3_bind_int64(stmt, 3, msg.GetAuthor().GetId().ToInt())) {
		auto content = msg.GetContent();
		if (   SQLITE_OK   == (content.empty() ? sqlite3_bind_null(stmt, 4) : sqlite3_bind_text64(stmt, 4, content.data(), content.size(), SQLITE_STATIC, SQLITE_UTF8))
		    && SQLITE_DONE == sqlite3_step(stmt)) {
			co_return;
	}	}
	throw xDatabaseError();
}
cTask<std::optional<message_entry>>
cDatabase::GetMessage(const cSnowflake& id) {
	co_await resume_on_db_strand();
	auto stmt = sqlite3_prepare(QUERY_GET_MESSAGE);
	if (stmt && SQLITE_OK == sqlite3_bind_int64(stmt, 1, id.ToInt())) {
		std::optional<message_entry> result;
		switch (sqlite3_step(stmt)) {
			case SQLITE_ROW: {
				auto &msg = result.emplace(
					id,
					sqlite3_column_int64(stmt, 0),
					sqlite3_column_int64(stmt, 1)
				);
				if (auto text = sqlite3_column_text(stmt, 2))
					msg.content = (const char *) text;
			}
			case SQLITE_DONE:
				co_return result;
		}
	}
	throw xDatabaseError();
}
cTask<std::optional<message_entry>>
cDatabase::UpdateMessage(const cSnowflake& id, std::string_view content) {
	auto result = co_await GetMessage(id);
	if (!result)
		co_return result;
	sqlite3_stmt_ptr stmt = sqlite3_prepare("UPDATE messages SET content=? WHERE id IS ?;");
	if (   stmt
	    && SQLITE_OK == (content.empty() ? sqlite3_bind_null(stmt, 1) : sqlite3_bind_text64(stmt, 1, content.data(), content.size(), SQLITE_STATIC, SQLITE_UTF8))
		&& SQLITE_OK == sqlite3_bind_int64(stmt, 2, id.ToInt())
		&& SQLITE_DONE == sqlite3_step(stmt)) {
		co_return result;
	}
	throw xDatabaseError();
}
cTask<std::optional<message_entry>>
cDatabase::DeleteMessage(const cSnowflake& id) {
	co_await resume_on_db_strand();
	auto stmt = sqlite3_prepare(QUERY_DELETE_MESSAGE);
	if (stmt && SQLITE_OK == sqlite3_bind_int64(stmt, 1, id.ToInt())) {
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
	auto stmt = sqlite3_prepare(query);
	if (!stmt)
		throw xDatabaseError();
	/* Bind ids to the statement */
	for (int i = 0; i < ids.size(); ++i) {
		if (SQLITE_OK != sqlite3_bind_int64(stmt, i + 1, ids[i].ToInt()))
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
	auto stmt = sqlite3_prepare(QUERY_CLEANUP_MESSAGES);
	if (   stmt
	    && SQLITE_OK   == sqlite3_bind_int64(stmt, 1, duration_cast<milliseconds>(system_clock::now() - sys_days(2015y/1/1) - 15 * 24h).count())
	    && SQLITE_DONE == sqlite3_step(stmt)) {
		co_return;
	}
	throw xDatabaseError();
}

cTask<>
cDatabase::RegisterTemporaryBan(crefUser user, std::chrono::sys_days expires_at) {
	if (expires_at == std::chrono::sys_days{})
		return RemoveTemporaryBan(user);
	return [](crefUser user, std::chrono::sys_days expires_at) -> cTask<> {
		co_await resume_on_db_strand();
		auto stmt = sqlite3_prepare(QUERY_REGISTER_TEMPORARY_BAN);
		if (stmt
		    && SQLITE_OK == sqlite3_bind_int64(stmt, 1, user.GetId().ToInt())
		    && SQLITE_OK == sqlite3_bind_int64(stmt, 2, expires_at.time_since_epoch().count())
		    && SQLITE_DONE == sqlite3_step(stmt)) {
			co_return;
		}
		throw xDatabaseError();
	} (user, expires_at);
}
cTask<>
cDatabase::RemoveTemporaryBan(crefUser user) {
	co_await resume_on_db_strand();
	auto stmt = sqlite3_prepare(QUERY_REMOVE_TEMPORARY_BAN);
	if (stmt
	    && SQLITE_OK == sqlite3_bind_int64(stmt, 1, user.GetId().ToInt())
	    && SQLITE_DONE == sqlite3_step(stmt)) {
		co_return;
	}
	throw xDatabaseError();
}