#include "Transaction.h"

void
cTransaction::Begin(std::error_code& ec) noexcept {
	/* If autocommit mode is off, then a transaction has begun; Treat this as a no-op */
	if (m_conn && !sqlite3_get_autocommit(m_conn))
		return ec.clear();
	/* Begin the transaction statement */
	auto[stmt, _] = m_conn.prepare("BEGIN TRANSACTION;", ec);
	/* Execute the statement while no error is set */
	while (!ec && stmt.step(ec));
}

void
cTransaction::Commit(std::error_code& ec) noexcept {
	/* If autocommit mode is on, then no transaction is in progress; Treat this as a no-op */
	if (m_conn && sqlite3_get_autocommit(m_conn))
		return ec.clear();
	/* Begin the commit statement */
	auto[stmt, _] = m_conn.prepare("COMMIT;", ec);
	/* Execute the statement while no error is set */
	while (!ec && stmt.step(ec));
}

void
cTransaction::Rollback(std::error_code& ec) noexcept {
	/* If autocommit mode is on, then a transaction isn't in progress */
	if (m_conn && sqlite3_get_autocommit(m_conn))
		return ec.clear();
	/* Begin the rollback statement */
	auto[stmt, _] = m_conn.prepare("ROLLBACK;", ec);
	/* Execute the statement while no error is set */
	while (!ec && stmt.step(ec));
}

sqlite::connection
cTransaction::ReleaseConnection(std::error_code& ec) noexcept {
	/* Rollback the transaction before returning, unless explicitly committed beforehand */
	Rollback(ec);
	return sqlite::connection(std::exchange(m_conn, {}));
}

void
cTransaction::Begin() {
	std::error_code ec;
	Begin(ec);
	if (ec)
		throw std::system_error(ec, m_conn.errmsg());
}

void
cTransaction::Commit() {
	std::error_code ec;
	Commit(ec);
	if (ec)
		throw std::system_error(ec, m_conn.errmsg());
}

void
cTransaction::Rollback() {
	std::error_code ec;
	Rollback(ec);
	if (ec)
		throw std::system_error(ec, m_conn.errmsg());
}

sqlite::connection
cTransaction::ReleaseConnection() {
	std::error_code ec;
	if (auto result = ReleaseConnection(ec); !ec)
		return result;
	throw std::system_error(ec, m_conn.errmsg());
}