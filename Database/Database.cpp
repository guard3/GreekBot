#include "Database.h"
#include "Queries.h"
#include "SQLite.h"
#include "Utils.h"
#include <thread>
#include <boost/asio/defer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
/* ========== The global sqlite connection ========================================================================== */
static sqlite::connection g_db;
/* ================================================================================================================== */
sqlite::connection
cDatabase::CreateInstance() {
	/* Open the database connection; NOMUTEX since we make sure to use the connection in a strand */
	return g_db ? std::move(g_db) : sqlite::connection(cUtils::GetExecutablePath().replace_filename("database.db"), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX);
}
/* ========== The database instance constructor which initializes the global sqlite connection ====================== */
void cDatabase::Initialize() noexcept {
	try {
		/* Open the database connection */
		g_db = CreateInstance();
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
/* ========== A custom io_context that runs on one strand and never runs out of work ================================ */
namespace net = boost::asio;
namespace sys = boost::system;
class db_context : public net::io_context {
	net::executor_work_guard<net::io_context::executor_type> m_work_guard;
	std::thread m_work_thread;
public:
	db_context() : net::io_context(1), m_work_guard(net::make_work_guard(*this)), m_work_thread(&db_context::run, this) {}
	~db_context() {
		m_work_guard.reset();
		m_work_thread.join();
	}
} static g_db_ctx;

[[nodiscard]]
auto resume_on_db_strand() {
	struct awaitable {
		bool await_ready() noexcept {
			return g_db_ctx.get_executor().running_in_this_thread();
		}
		void await_suspend(std::coroutine_handle<> h) {
			net::defer(g_db_ctx, h);
		}
		void await_resume() noexcept {}
	};
	return awaitable{};
}

[[nodiscard]]
auto wait_on_db_strand(std::chrono::milliseconds duration) {
	struct awaitable {
		net::steady_timer timer;

		bool await_ready() noexcept {
			return false;
		}
		void await_suspend(std::coroutine_handle<> h) {
			timer.async_wait([h] (const sys::error_code&) { h(); });
		}
		void await_resume() noexcept {}
	};
	return awaitable{ net::steady_timer(g_db_ctx, duration) };
};

cTask<>
cDatabase::ResumeOnDatabaseStrand() {
	co_await resume_on_db_strand();
}
cTask<>
cDatabase::Wait(std::chrono::milliseconds duration) {
	co_await wait_on_db_strand(duration);
}
/* ================================================================================================================== */
cTask<cTransaction>
cDatabase::BorrowDatabase() {
	co_await resume_on_db_strand();
	co_return cTransaction(CreateInstance());
}

cTask<>
cDatabase::ReturnDatabase(cTransaction txn) {
	co_await resume_on_db_strand();
	g_db = txn.ReleaseConnection();
}
/* ================================================================================================================== */
cTask<uint64_t>
cDatabase::WC_RegisterMember(const cMember& member) {
	co_await resume_on_db_strand();
	/* Prepare statement */
	auto conn = CreateInstance();
	auto[stmt, _] = conn.prepare(QUERY_WC_REG_MBR);
	g_db = std::move(conn);
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

	auto conn = CreateInstance();
	auto[stmt, _] = conn.prepare(QUERY_WC_GET_MSG);
	g_db = std::move(conn);
	stmt.bind(1, member.GetUser().GetId());

	co_return stmt.step() ? stmt.column_int(0) : -1;
}
cTask<>
cDatabase::WC_EditMessage(int64_t msg_id) {
	co_await resume_on_db_strand();

	auto conn = CreateInstance();
	auto[stmt, _] = conn.prepare(QUERY_WC_EDT_MSG);
	g_db = std::move(conn);
	stmt.bind(1, msg_id);

	stmt.step();
}
cTask<uint64_t>
cDatabase::WC_DeleteMember(const cUser& user) {
	co_await resume_on_db_strand();

	auto conn = CreateInstance();
	auto[stmt, _] = conn.prepare(QUERY_WC_DEL_MBR);
	g_db = std::move(conn);
	stmt.bind(1, user.GetId());

	co_return stmt.step() ? static_cast<uint64_t>(stmt.column_int(0)) : 0;
}
cTask<>
cDatabase::WC_UpdateMessage(const cUser& user, const cMessage& msg) {
	co_await resume_on_db_strand();

	auto conn = CreateInstance();
	auto[stmt, _] = conn.prepare(QUERY_WC_UPD_MSG);
	g_db = std::move(conn);
	stmt.bind(1, user.GetId());
	stmt.bind(2, msg.GetId());

	stmt.step();
}

cTask<>
cDatabase::RegisterTemporaryBan(crefUser user, std::chrono::sys_days expires_at) {
	if (expires_at == std::chrono::sys_days{})
		return RemoveTemporaryBan(user);
	return [](crefUser user, std::chrono::sys_days expires_at) -> cTask<> {
		co_await resume_on_db_strand();
		auto conn = cDatabase::CreateInstance();
		auto[stmt, _] = conn.prepare(QUERY_REGISTER_TEMPORARY_BAN);
		g_db = std::move(conn);
		stmt.bind(1, user.GetId());
		stmt.bind(2, expires_at.time_since_epoch().count());
		stmt.step();
	} (user, expires_at);
}
cTask<std::vector<cSnowflake>>
cDatabase::GetExpiredTemporaryBans() {
	using namespace std::chrono;
	co_await resume_on_db_strand();
	auto conn = CreateInstance();
	auto[stmt, _] = conn.prepare("SELECT user_id FROM tempbans WHERE ? >= expires_at;");
	g_db = std::move(conn);
	stmt.bind(1, floor<days>(system_clock::now()).time_since_epoch().count());
	std::vector<cSnowflake> result;
	while (stmt.step())
		result.emplace_back(stmt.column_int(0));
	co_return result;
}
cTask<>
cDatabase::RemoveTemporaryBan(crefUser user) {
	co_await resume_on_db_strand();
	auto conn = CreateInstance();
	auto[stmt, _] = conn.prepare(QUERY_REMOVE_TEMPORARY_BAN);
	g_db = std::move(conn);
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
	auto conn = CreateInstance();
	auto[stmt, _] = conn.prepare(query);
	g_db = std::move(conn);
	for (std::size_t i = 0; i < user_ids.size(); ++i)
		stmt.bind(i + 1, user_ids[i]);
	stmt.step();
}