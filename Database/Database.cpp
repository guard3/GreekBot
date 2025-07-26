#include "Database.h"
#include "Queries.h"
#include "SQLite.h"
#include "Utils.h"
#include <thread>
#include <boost/asio/post.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>

namespace net = boost::asio;
using namespace std::chrono_literals;

/**
 * The global sqlite connection instance
 */
static sqlite::connection dbConn;

sqlite::connection
static dbCreateInstance() {
	/* Open the database connection; NOMUTEX since we make sure to use the connection in a strand */
	return dbConn ? std::move(dbConn) : sqlite::connection(cUtils::GetExecutablePath().replace_filename("database.db"), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX);
}

void
dbInitialize() noexcept {
	try {
		/* Open the database connection */
		dbConn = dbCreateInstance();
		/* Execute the init statements */
		for (auto ret = dbConn.prepare(QUERY_INIT); ret.stmt; ret = dbConn.prepare(ret.tail))
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

/**
 * A custom io_context that runs on one strand and never runs out of work
 */
class dbContext : public net::io_context {
	net::executor_work_guard<net::io_context::executor_type> m_work_guard;
	std::thread m_work_thread;
public:
	dbContext() : net::io_context(1), m_work_guard(net::make_work_guard(*this)), m_work_thread(&net::io_context::run, this) {}
	~dbContext() {
		m_work_guard.reset();
		m_work_thread.join();
	}
} static dbCtx;

/**
 * Resume the execution of the caller coroutine to the database strand
 * @return
 */
[[nodiscard]]
static auto dbResume() {
	struct awaitable {
		bool await_ready() noexcept {
			return dbCtx.get_executor().running_in_this_thread();
		}
		void await_suspend(std::coroutine_handle<> h) {
			net::post(dbCtx, h);
		}
		void await_resume() noexcept {}
	};
	return awaitable{};
}

/**
 * Wait and resume execution of the caller coroutine to the database strand
 * @param duration
 * @return
 */
[[nodiscard]]
static auto dbWait(std::chrono::milliseconds duration) {
	struct awaitable {
		net::steady_timer timer;

		bool await_ready() noexcept {
			return false;
		}
		void await_suspend(std::coroutine_handle<> h) {
			timer.async_wait([h] (auto&&) { h.resume(); });
		}
		void await_resume() noexcept {}
	};
	return awaitable{ net::steady_timer(dbCtx, duration) };
}
/* ================================================================================================================== */

void
refTransaction::begin_impl(cTransactionType type, std::error_code& ec) noexcept {
	/* If autocommit mode is off, then a transaction has begun; Treat this as a no-op */
	if (!m_conn || !m_conn.autocommit())
		return ec.clear();
	/* Begin the transaction statement */
	auto[stmt, _] = m_conn.prepare(type.m_query, ec);
	/* Execute the statement while no error is set */
	while (!ec && stmt.step(ec));
}

void
refTransaction::commit_impl(std::error_code& ec) noexcept {
	/* If autocommit mode is on, then no transaction is in progress; Treat this as a no-op */
	if (!m_conn || m_conn.autocommit())
		return ec.clear();
	/* Begin the commit statement */
	auto[stmt, _] = m_conn.prepare("COMMIT;", ec);
	/* Execute the statement while no error is set */
	while (!ec && stmt.step(ec));
}

void
refTransaction::rollback_impl(std::error_code& ec) noexcept {
	/* If autocommit mode is on, then a transaction isn't in progress */
	if (!m_conn || m_conn.autocommit())
		return ec.clear();
	/* Begin the rollback statement */
	auto[stmt, _] = m_conn.prepare("ROLLBACK;", ec);
	/* Execute the statement while no error is set */
	while (!ec && stmt.step(ec));
}

cTask<>
refTransaction::Begin(cTransactionType type, std::error_code& ec) {
	/* Begin transaction */
	co_await dbResume();
	begin_impl(type, ec);
	/* Retry if there are busy errors */
	for (int i = 0; ec == sqlite::error::busy && i < 100; ++i) {
		/* Attempt to roll back the transaction */
		if (rollback_impl(ec); ec)
			break;
		/* Wait a bit before retrying */
		co_await dbWait(100ms);
		begin_impl(type, ec);
	}
}

cTask<>
refTransaction::Commit(std::error_code& ec) {
	/* Commit transaction */
	co_await dbResume();
	commit_impl(ec);
	/* Retry if there are busy errors */
	for (int i = 0; ec == sqlite::error::busy && i < 100; ++i) {
		/* Wait a bit before retrying; don't roll back the transaction */
		co_await dbWait(100ms);
		commit_impl(ec);
	}
}

cTask<>
refTransaction::Rollback(std::error_code& ec) {
	co_await dbResume();
	rollback_impl(ec);
}

cTask<>
refTransaction::Begin(cTransactionType type) {
	std::error_code ec;
	if (co_await Begin(type, ec); ec)
		throw std::system_error(ec, m_conn.errmsg());
}

cTask<>
refTransaction::Commit() {
	std::error_code ec;
	if (co_await Commit(ec); ec)
		throw std::system_error(ec, m_conn.errmsg());
}

cTask<>
refTransaction::Rollback() {
	std::error_code ec;
	if (co_await Rollback(ec); ec)
		throw std::system_error(ec, m_conn.errmsg());
}

void refTransaction::Close() {
	if (m_conn) {
		net::dispatch(dbCtx, [conn = std::exchange(m_conn, {})] mutable {
			std::error_code ec;
			if (refTransaction(conn).rollback_impl(ec); ec)
				conn.close();
			else
				dbConn = sqlite::connection(conn);
		});
	}
}

cTask<cTransaction>
cTransaction::New() {
	co_await dbResume();
	co_return cTransaction(dbCreateInstance());
}

sqlite::connection
cTransaction::ReleaseConnection(std::error_code& ec) noexcept {
	rollback_impl(ec);
	return ec ? sqlite::connection{} : sqlite::connection(std::exchange(m_conn, {}));
}

sqlite::connection
cTransaction::ReleaseConnection() {
	std::error_code ec;
	if (rollback_impl(ec); ec)
		throw std::system_error(ec, m_conn.errmsg());
	return sqlite::connection(std::exchange(m_conn, {}));
}

/**
 * DAO helper functions for switching to the database strand
 */
cTask<>
cBaseDAO::resume_on_db_strand() {
	co_await dbResume();
}
cTask<>
cBaseDAO::wait_on_db_strand(std::chrono::milliseconds duration) {
	co_await dbWait(duration);
}