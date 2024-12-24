#include "Database.h"
#include "Queries.h"
#include "SQLite.h"
#include "Utils.h"
#include <thread>
#include <boost/asio/defer.hpp>
#include <boost/asio/io_context.hpp>
/* ========== The global sqlite connection ========================================================================== */
static sqlite::connection g_db;
/* ========== The database exception which uses the global sqlite connection's error message and error code ========= */
[[noreturn]]
static void throw_database_error() {
	throw std::system_error(sqlite3_extended_errcode(g_db), sqlite::error_category(), g_db.errmsg());
}
/* ========== The database instance constructor which initializes the global sqlite connection ====================== */
void cDatabase::Initialize() noexcept {
	try {
		/* Open the database connection; NOMUTEX since we make sure to use the connection in a strand */
		g_db.open(cUtils::GetExecutablePath().replace_filename("database.db"), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX);
		/* Execute the init statements */
		for (auto ret = g_db.prepare(QUERY_INIT); ret.stmt; ret = g_db.prepare(ret.tail))
			while(ret.stmt.step());
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
/* ================================================================================================================== */
cTask<std::uint64_t>
cDatabase::UpdateLeaderboard(const cMessage& msg) {
	using namespace std::chrono;
	co_await resume_on_db_strand();
	auto[stmt, _] = g_db.prepare(QUERY_UPDATE_LB);
	if (   SQLITE_OK  == sqlite3_bind_int64(stmt, 1, msg.GetAuthor().GetId().ToInt())
	    && SQLITE_OK  == sqlite3_bind_int64(stmt, 2, floor<seconds>(msg.GetTimestamp()).time_since_epoch().count())) {
		co_return stmt.step() ? sqlite3_column_int64(stmt, 0) : 0;
	}
	throw_database_error();
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
	throw_database_error();
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
	throw_database_error();
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
	throw_database_error();
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
	throw_database_error();
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
	throw_database_error();
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
	throw_database_error();
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
	throw_database_error();
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
	throw_database_error();
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
	throw_database_error();
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
	throw_database_error();
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
	throw_database_error();
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
	throw_database_error();
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
	throw_database_error();
}
cTask<std::optional<leaderboard_entry>>
cDatabase::GetUserRank(const cUser& user) {
	co_await resume_on_db_strand();
	auto[stmt, _] = g_db.prepare(QUERY_GET_RANK);
	if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, user.GetId().ToInt())) {
		std::optional<leaderboard_entry> result;
		if (stmt.step()) {
			result.emplace(
				user.GetId(),
				sqlite3_column_int64(stmt, 0),
				sqlite3_column_int64(stmt, 1),
				sqlite3_column_int64(stmt, 2)
			);
		}
		co_return result;
	}
	throw_database_error();
}
cTask<std::vector<leaderboard_entry>>
cDatabase::GetTop10() {
	co_await resume_on_db_strand();
	auto[stmt, _] = g_db.prepare(QUERY_GET_TOP_10);
	std::vector<leaderboard_entry> result;
	for (result.reserve(10); stmt.step();) {
		result.emplace_back(
			sqlite3_column_int64(stmt, 0),
			sqlite3_column_int64(stmt, 1),
			sqlite3_column_int64(stmt, 2),
			sqlite3_column_int64(stmt, 3)
		);
	}
	co_return result;
}
cTask<>
cDatabase::RegisterMessage(const cMessage& msg) {
	co_await resume_on_db_strand();
	auto[stmt, _] = g_db.prepare(QUERY_REGISTER_MESSAGE);
	if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, msg.GetId().ToInt())
	 && SQLITE_OK == sqlite3_bind_int64(stmt, 2, msg.GetChannelId().ToInt())
	 && SQLITE_OK == sqlite3_bind_int64(stmt, 3, msg.GetAuthor().GetId().ToInt())) {
		auto content = msg.GetContent();
		if (SQLITE_OK == (content.empty() ? sqlite3_bind_null(stmt, 4) : sqlite3_bind_text64(stmt, 4, content.data(), content.size(), SQLITE_STATIC, SQLITE_UTF8))) {
			stmt.step();
			co_return ;
		}
	}
	throw_database_error();
}
cTask<std::optional<message_entry>>
cDatabase::GetMessage(const cSnowflake& id) {
	co_await resume_on_db_strand();
	auto[stmt, _] = g_db.prepare(QUERY_GET_MESSAGE);
	if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, id.ToInt())) {
		std::optional<message_entry> result;
		if (stmt.step()) {
			auto& msg = result.emplace(
				id,
				sqlite3_column_int64(stmt, 0),
				sqlite3_column_int64(stmt, 1)
			);
			if (auto text = sqlite3_column_text(stmt, 2))
				msg.content = reinterpret_cast<const char*>(text);
		}
		co_return result;
	}
	throw_database_error();
}
cTask<std::optional<message_entry>>
cDatabase::UpdateMessage(const cSnowflake& id, std::string_view content) {
	auto result = co_await GetMessage(id);
	if (!result)
		co_return result;
	auto[stmt, _] = g_db.prepare("UPDATE messages SET content=? WHERE id IS ?;");
	if (SQLITE_OK == (content.empty() ? sqlite3_bind_null(stmt, 1) : sqlite3_bind_text64(stmt, 1, content.data(), content.size(), SQLITE_STATIC, SQLITE_UTF8))
	 && SQLITE_OK == sqlite3_bind_int64(stmt, 2, id.ToInt())) {
		stmt.step();
		co_return result;
	}
	throw_database_error();
}
cTask<std::optional<message_entry>>
cDatabase::DeleteMessage(const cSnowflake& id) {
	co_await resume_on_db_strand();
	auto[stmt, _] = g_db.prepare(QUERY_DELETE_MESSAGE);
	if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, id.ToInt())) {
		std::optional<message_entry> result;
		if (stmt.step()) {
			result.emplace(
				id,
				sqlite3_column_int64(stmt, 0),
				sqlite3_column_int64(stmt, 1)
			);
			if (auto text = sqlite3_column_text(stmt, 2))
				result->content = reinterpret_cast<const char*>(text);
		}
		co_return result;
	}
	throw_database_error();
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
	/* Bind ids to the statement */
	for (std::size_t i = 0; i < ids.size(); ++i) {
		if (SQLITE_OK != sqlite3_bind_int64(stmt, i + 1, ids[i].ToInt()))
			throw_database_error();
	}
	/* Retrieve result */
	std::vector<message_entry> result;
	for (result.reserve(ids.size()); stmt.step();) {
		auto& entry = result.emplace_back(
			sqlite3_column_int64(stmt, 0),
			sqlite3_column_int64(stmt, 1),
			sqlite3_column_int64(stmt, 2)
		);
		if (auto text = sqlite3_column_text(stmt, 3))
			entry.content = reinterpret_cast<const char*>(text);
	}
	co_return result;
}
cTask<>
cDatabase::CleanupMessages() {
	using namespace std::chrono;
	using namespace std::chrono_literals;
	co_await resume_on_db_strand();
	auto[stmt, _] = g_db.prepare(QUERY_CLEANUP_MESSAGES);
	if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, duration_cast<milliseconds>(system_clock::now() - sys_days(2015y/1/1) - 15 * 24h).count())) {
		stmt.step();
		co_return;
	}
	throw_database_error();
}

cTask<>
cDatabase::RegisterTemporaryBan(crefUser user, std::chrono::sys_days expires_at) {
	if (expires_at == std::chrono::sys_days{})
		return RemoveTemporaryBan(user);
	return [](crefUser user, std::chrono::sys_days expires_at) -> cTask<> {
		co_await resume_on_db_strand();
		auto[stmt, _] = g_db.prepare(QUERY_REGISTER_TEMPORARY_BAN);
		if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, user.GetId().ToInt())
		 && SQLITE_OK == sqlite3_bind_int64(stmt, 2, expires_at.time_since_epoch().count())) {
			stmt.step();
			co_return;
		}
		throw_database_error();
	} (user, expires_at);
}
cTask<std::vector<cSnowflake>>
cDatabase::GetExpiredTemporaryBans() {
	using namespace std::chrono;
	co_await resume_on_db_strand();
	auto[stmt, _] = g_db.prepare("SELECT user_id FROM tempbans WHERE ? >= expires_at;");
	if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, floor<days>(system_clock::now()).time_since_epoch().count())) {
		std::vector<cSnowflake> result;
		while (stmt.step())
			result.emplace_back(sqlite3_column_int64(stmt, 0));
		co_return result;
	}
	throw_database_error();
}
cTask<>
cDatabase::RemoveTemporaryBan(crefUser user) {
	co_await resume_on_db_strand();
	auto[stmt, _] = g_db.prepare(QUERY_REMOVE_TEMPORARY_BAN);
	if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, user.GetId().ToInt())) {
		stmt.step();
		co_return;
	}
	throw_database_error();
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
	for (std::size_t i = 0; i < user_ids.size(); ++i) {
		if (SQLITE_OK != sqlite3_bind_int64(stmt, i + 1, user_ids[i].ToInt()))
			throw_database_error();
	}
	stmt.step();
}