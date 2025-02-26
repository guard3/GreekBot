#include "Database.h"
#include "Queries.h"
#include "SQLite.h"
#include "Utils.h"
#include <thread>
#include <boost/asio/defer.hpp>
#include <boost/asio/io_context.hpp>
/* ========== The global sqlite connection ========================================================================== */
static sqlite::connection g_db;
/* ================================================================================================================== */
sqlite::connection
cDatabase::CreateInstance() {
	/* Open the database connection; NOMUTEX since we make sure to use the connection in a strand */
	sqlite::connection conn(cUtils::GetExecutablePath().replace_filename("database.db"), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX);
	/* Execute the init statements */
	for (auto ret = conn.prepare(QUERY_INIT); ret.stmt; ret = conn.prepare(ret.tail))
		while(ret.stmt.step());
	return conn;
}
/* ========== The database instance constructor which initializes the global sqlite connection ====================== */
void cDatabase::Initialize() noexcept {
	try {
		/* Open the database connection; NOMUTEX since we make sure to use the connection in a strand */
		g_db = CreateInstance();
		/* Print confirmation message */
		cUtils::PrintDbg("Database initialized successfully");
		return;
	} catch (const std::exception& e) {
		cUtils::PrintErr("Database initialization error: {}", e.what());
	} catch (...) {
		cUtils::PrintErr("Database initialization error: {}", "An unknown error occurred");
	}
	std::exit(EXIT_FAILURE);
}
/* ========== An awaitable which resumes a coroutine to the sqlite connection strand ================================ */
namespace net = boost::asio;
class resume_on_db_strand {
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

cTask<>
cDatabase::ResumeOnDatabaseStrand() {
	co_await resume_on_db_strand();
}
/* ================================================================================================================== */
cTask<uint64_t>
cDatabase::WC_RegisterMember(const cMember& member) {
	co_await resume_on_db_strand();
	/* Prepare statement */
	auto[stmt, _] = g_db.prepare(QUERY_WC_REG_MBR);
	stmt.bind(1, member.GetUser()->GetId());
	stmt.bind(2, member.JoinedAt().time_since_epoch().count());
	/* Make sure the statement returns at least one row */
	if (!stmt.step())
		throw std::system_error(SQLITE_INTERNAL, sqlite::error_category());
	co_return static_cast<uint64_t>(stmt.column_int(0));
}
cTask<int64_t>
cDatabase::WC_GetMessage(const cMemberUpdate& member) {
	co_await resume_on_db_strand();

	auto[stmt, _] = g_db.prepare(QUERY_WC_GET_MSG);
	stmt.bind(1, member.GetUser().GetId());

	co_return stmt.step() ? stmt.column_int(0) : -1;
}
cTask<>
cDatabase::WC_EditMessage(int64_t msg_id) {
	co_await resume_on_db_strand();

	auto[stmt, _] = g_db.prepare(QUERY_WC_EDT_MSG);
	stmt.bind(1, msg_id);

	stmt.step();
}
cTask<uint64_t>
cDatabase::WC_DeleteMember(const cUser& user) {
	co_await resume_on_db_strand();

	auto[stmt, _] = g_db.prepare(QUERY_WC_DEL_MBR);
	stmt.bind(1, user.GetId());

	co_return stmt.step() ? static_cast<uint64_t>(stmt.column_int(0)) : 0;
}
cTask<>
cDatabase::WC_UpdateMessage(const cUser& user, const cMessage& msg) {
	co_await resume_on_db_strand();

	auto[stmt, _] = g_db.prepare(QUERY_WC_UPD_MSG);
	stmt.bind(1, user.GetId());
	stmt.bind(2, msg.GetId());

	stmt.step();
}
cTask<int64_t>
cDatabase::SB_GetMessageAuthor(const cSnowflake& msg_id) {
	co_await resume_on_db_strand();

	auto[stmt, _] = g_db.prepare(QUERY_SB_GET_MESSAGE_AUTHOR);
	stmt.bind(1, msg_id);

	co_return stmt.step() ? stmt.column_int(0) : 0;
}
cTask<std::pair<int64_t, int64_t>>
cDatabase::SB_RegisterReaction(const cSnowflake& msg_id, const cSnowflake& author_id) {
	co_await resume_on_db_strand();
	/* Prepare statement */
	auto[stmt, _] = g_db.prepare(QUERY_SB_REGISTER_REACTION);
	stmt.bind(1, msg_id);
	stmt.bind(2, author_id);
	/* Make sure the statement returns at least one row */
	if (!stmt.step())
		throw std::system_error(SQLITE_INTERNAL, sqlite::error_category());
	co_return { stmt.column_int(0), stmt.column_int(1) };
}
cTask<>
cDatabase::SB_RegisterMessage(const cSnowflake& msg_id, const cSnowflake& sb_msg_id) {
	co_await resume_on_db_strand();
	auto[stmt, _] = g_db.prepare(QUERY_SB_REGISTER_MESSAGE);
	stmt.bind(1, msg_id);
	stmt.bind(2, sb_msg_id);
	stmt.step();
}
cTask<std::pair<int64_t, int64_t>>
cDatabase::SB_RemoveReaction(const cSnowflake& msg_id) {
	co_await resume_on_db_strand();
	auto[stmt, _] = g_db.prepare(QUERY_SB_REMOVE_REACTION);
	stmt.bind(1, msg_id);
	co_return stmt.step() ? std::pair<int64_t, int64_t>{ stmt.column_int(0), stmt.column_int(1) } :
	                        std::pair<int64_t, int64_t>{};
}
cTask<>
cDatabase::SB_RemoveMessage(const cSnowflake& msg_id) {
	co_await resume_on_db_strand();
	auto[stmt, _] = g_db.prepare(QUERY_SB_REMOVE_MESSAGE);
	stmt.bind(1, msg_id);
	stmt.step();
}
cTask<int64_t>
cDatabase::SB_RemoveAll(const cSnowflake& msg_id) {
	co_await resume_on_db_strand();
	auto[stmt, _] = g_db.prepare(QUERY_SB_REMOVE_ALL);
	stmt.bind(1, msg_id);
	co_return stmt.step() ? stmt.column_int(0) : 0;
}
cTask<std::vector<starboard_entry>>
cDatabase::SB_GetTop10(int threshold) {
	co_await resume_on_db_strand();
	auto[stmt, _] = g_db.prepare(QUERY_SB_GET_TOP_10);
	stmt.bind(1, threshold);
	std::vector<starboard_entry> result;
	result.reserve(10);
	for (int64_t rank = 1; stmt.step(); ++rank) {
		result.emplace_back(
			stmt.column_int(0),
			stmt.column_int(1),
			stmt.column_int(2),
			stmt.column_int(3),
			rank
		);
	}
	co_return result;
}
cTask<std::vector<starboard_entry>>
cDatabase::SB_GetRank(const cUser& user, int threshold) {
	co_await resume_on_db_strand();

	auto[stmt, _] = g_db.prepare(QUERY_SB_GET_RANK);
	stmt.bind(1, user.GetId());
	stmt.bind(2, threshold);

	std::vector<starboard_entry> result;
	if (stmt.step()) {
		result.emplace_back(
			stmt.column_int(0),
			stmt.column_int(1),
			stmt.column_int(2),
			stmt.column_int(3),
			stmt.column_int(4)
		);
	}
	co_return result;
}

cTask<std::optional<message_entry>>
cDatabase::DeleteMessage(const cSnowflake& id) {
	co_await resume_on_db_strand();
	auto[stmt, _] = g_db.prepare(QUERY_DELETE_MESSAGE);
	stmt.bind(1, id);
	std::optional<message_entry> result;
	if (stmt.step()) {
		result.emplace(
			id,
			stmt.column_int(0),
			stmt.column_int(1)
		);
		if (auto text = stmt.column_text(2))
			result->content = text;
	}
	co_return result;
}
cTask<std::vector<message_entry>>
cDatabase::DeleteMessages(std::span<const cSnowflake> ids) {
	co_await resume_on_db_strand();
	if (ids.empty())
		co_return {};
	/* Make query */
	std::string query = "DELETE FROM messages WHERE id IN (?";
	for (std::size_t i = 1; i < ids.size(); ++i)
		query += ",?";
	query += ") RETURNING *;";
	/* Prepare statement */
	auto[stmt, _] = g_db.prepare(query);
	for (std::size_t i = 0; i < ids.size(); ++i)
		stmt.bind(i + 1, ids[i]);
	/* Retrieve result */
	std::vector<message_entry> result;
	for (result.reserve(ids.size()); stmt.step();) {
		auto& entry = result.emplace_back(
			stmt.column_int(0),
			stmt.column_int(1),
			stmt.column_int(2)
		);
		if (auto text = stmt.column_text(3))
			entry.content = text;
	}
	co_return result;
}
cTask<>
cDatabase::CleanupMessages() {
	using namespace std::chrono;
	using namespace std::chrono_literals;
	co_await resume_on_db_strand();
	auto[stmt, _] = g_db.prepare(QUERY_CLEANUP_MESSAGES);
	stmt.bind(1, duration_cast<milliseconds>(system_clock::now() - sys_days(2015y/1/1) - 15 * 24h).count());
	stmt.step();
}

cTask<>
cDatabase::RegisterTemporaryBan(crefUser user, std::chrono::sys_days expires_at) {
	if (expires_at == std::chrono::sys_days{})
		return RemoveTemporaryBan(user);
	return [](crefUser user, std::chrono::sys_days expires_at) -> cTask<> {
		co_await resume_on_db_strand();
		auto[stmt, _] = g_db.prepare(QUERY_REGISTER_TEMPORARY_BAN);
		stmt.bind(1, user.GetId());
		stmt.bind(2, expires_at.time_since_epoch().count());
		stmt.step();
	} (user, expires_at);
}
cTask<std::vector<cSnowflake>>
cDatabase::GetExpiredTemporaryBans() {
	using namespace std::chrono;
	co_await resume_on_db_strand();
	auto[stmt, _] = g_db.prepare("SELECT user_id FROM tempbans WHERE ? >= expires_at;");
	stmt.bind(1, floor<days>(system_clock::now()).time_since_epoch().count());
	std::vector<cSnowflake> result;
	while (stmt.step())
		result.emplace_back(stmt.column_int(0));
	co_return result;
}
cTask<>
cDatabase::RemoveTemporaryBan(crefUser user) {
	co_await resume_on_db_strand();
	auto[stmt, _] = g_db.prepare(QUERY_REMOVE_TEMPORARY_BAN);
	stmt.bind(1, user.GetId());
	stmt.step();
}
cTask<>
cDatabase::RemoveTemporaryBans(std::span<const cSnowflake> user_ids) {
	if (user_ids.empty())
		co_return;
	co_await resume_on_db_strand();
	std::string query = "DELETE FROM tempbans WHERE user_id IN (?";
	for (std::size_t i = 1; i < user_ids.size(); ++i)
		query += ",?";
	query += ");";
	auto[stmt, _] = g_db.prepare(query);
	for (std::size_t i = 0; i < user_ids.size(); ++i)
		stmt.bind(i + 1, user_ids[i]);
	stmt.step();
}