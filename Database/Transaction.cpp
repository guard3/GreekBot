#include "Database.h"

using namespace std::chrono_literals;

const cTransactionType cTransactionType::Deferred{  "BEGIN DEFERRED;"  };
const cTransactionType cTransactionType::Immediate{ "BEGIN IMMEDIATE;" };

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
	begin_impl(type, ec);
	/* Retry if there are busy errors */
	for (int i = 0; ec == sqlite::error::busy && i < 100; ++i) {
		/* Attempt to roll back the transaction */
		if (rollback_impl(ec); ec)
			break;
		/* Wait a bit before retrying */
		co_await cDatabase::Wait(100ms);
		begin_impl(type, ec);
	}
}

cTask<>
refTransaction::Commit(std::error_code& ec) {
	/* Commit transaction */
	commit_impl(ec);
	/* Retry if there are busy errors */
	for (int i = 0; ec == sqlite::error::busy && i < 100; ++i) {
		/* Wait a bit before retrying; don't roll back the transaction */
		co_await cDatabase::Wait(100ms);
		commit_impl(ec);
	}
}

cTask<>
refTransaction::Rollback(std::error_code& ec) {
	co_return rollback_impl(ec);
}

sqlite::connection
cTransaction::ReleaseConnection(std::error_code& ec) noexcept {
	/* Rollback the transaction before returning, unless explicitly committed beforehand */
	rollback_impl(ec);
	return sqlite::connection(std::exchange(m_conn, {}));
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

sqlite::connection
cTransaction::ReleaseConnection() {
	std::error_code ec;
	if (auto result = ReleaseConnection(ec); !ec)
		return result;
	throw std::system_error(ec, m_conn.errmsg());
}