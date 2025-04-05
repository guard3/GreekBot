#ifndef GREEKBOT_TRANSACTION_H
#define GREEKBOT_TRANSACTION_H
#include "Coroutines.h"
#include "SQLite.h"

class cTransactionType {
	std::string_view m_query;
	consteval explicit cTransactionType(std::string_view query) noexcept : m_query(query) {}
public:
	static const cTransactionType Deferred;
	static const cTransactionType Immediate;

	friend class refTransaction;
};

class refTransaction {
protected:
	sqlite::connection_ref m_conn;
	void begin_impl(cTransactionType type, std::error_code& ec) noexcept;
	void commit_impl(std::error_code& ec) noexcept;
	void rollback_impl(std::error_code& ec) noexcept;

public:
	explicit refTransaction(sqlite::connection_ref conn) noexcept : m_conn(conn) {}

	sqlite::connection_ref GetConnection() noexcept { return m_conn; }

	[[nodiscard]] cTask<> Begin(cTransactionType type, std::error_code& ec);
	[[nodiscard]] cTask<> Commit(std::error_code& ec);
	[[nodiscard]] cTask<> Rollback(std::error_code& ec);

	[[nodiscard]] cTask<> Begin(cTransactionType type = cTransactionType::Deferred);
	[[nodiscard]] cTask<> Commit();
	[[nodiscard]] cTask<> Rollback();
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

	sqlite::connection ReleaseConnection(std::error_code& ec) noexcept;
	sqlite::connection ReleaseConnection();
};

#endif //GREEKBOT_TRANSACTION_H
