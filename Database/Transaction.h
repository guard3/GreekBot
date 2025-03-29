#ifndef GREEKBOT_TRANSACTION_H
#define GREEKBOT_TRANSACTION_H
#include "SQLite.h"

class cTransaction {
	sqlite::connection m_conn;
public:
	/* Become the owner of a database connection since only one transaction is allowed per connection */
	explicit cTransaction(sqlite::connection conn) noexcept : m_conn(std::move(conn)) {}

	void Begin(std::error_code&) noexcept;
	void Commit(std::error_code&) noexcept;
	void Rollback(std::error_code&) noexcept;
	sqlite::connection ReleaseConnection(std::error_code& ec) noexcept;

	void Begin();
	void Commit();
	void Rollback();
	sqlite::connection ReleaseConnection();

	sqlite::connection_ref GetConnection() noexcept { return m_conn; }
	void SetConnection(sqlite::connection conn) noexcept { m_conn = std::move(conn); }
};

#endif //GREEKBOT_TRANSACTION_H
