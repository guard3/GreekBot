#ifndef GREEKBOT_TRANSACTION_H
#define GREEKBOT_TRANSACTION_H
#include "SQLite.h"

class refTransaction {
protected:
	sqlite::connection_ref m_conn;
public:
	explicit refTransaction(sqlite::connection_ref conn) noexcept : m_conn(conn) {}

	sqlite::connection_ref GetConnection() noexcept { return m_conn; }
};

class cTransaction : public refTransaction {
public:
	/* Become the owner of a database connection since only one transaction is allowed per connection */
	explicit cTransaction(sqlite::connection conn) noexcept : refTransaction(conn.release()) {}

	cTransaction(const cTransaction&) = delete;
	cTransaction(cTransaction&& o) noexcept : refTransaction(std::exchange(o.m_conn, {})) {}

	~cTransaction() {
		// TODO: return the connection back to the global
		m_conn.close();
	}

	cTransaction& operator=(cTransaction o) noexcept {
		std::swap<refTransaction>(*this, o);
		return *this;
	}

	void Begin(std::error_code&) noexcept;
	void Commit(std::error_code&) noexcept;
	void Rollback(std::error_code&) noexcept;
	sqlite::connection ReleaseConnection(std::error_code& ec) noexcept;

	void Begin();
	void Commit();
	void Rollback();
	sqlite::connection ReleaseConnection();
};

#endif //GREEKBOT_TRANSACTION_H
