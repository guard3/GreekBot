#ifndef GREEKBOT_DATABASE_H
#define GREEKBOT_DATABASE_H
#include "Base.h"
#include "Coroutines.h"
#include "SQLite.h"

/**
 * Initialize the database file
 */
void
dbInitialize() noexcept;

/**
 * Make cSnowflake bindable to prepared statements as an int
 */
template<>
struct sqlite::binder<cSnowflake> {
	static void bind(statement_ref stmt, int index, const cSnowflake& id, std::error_code& ec) noexcept {
		stmt.bind(index, id.ToInt(), ec);
	}
};

/**
 * A type describing whether a write transaction is immediate or deferred
 */
class cTransactionType {
	std::string_view m_query;
	consteval explicit cTransactionType(std::string_view query) noexcept : m_query(query) {}
public:
	static const cTransactionType Deferred;
	static const cTransactionType Immediate;

	friend class refTransaction;
};

inline constexpr cTransactionType cTransactionType::Deferred{  "BEGIN DEFERRED;"  };
inline constexpr cTransactionType cTransactionType::Immediate{ "BEGIN IMMEDIATE;" };

/**
 * A non-owning reference to a transaction object
 */
class refTransaction {
protected:
	sqlite::connection_ref m_conn;
	void begin_impl(cTransactionType type, std::error_code& ec) noexcept;
	void commit_impl(std::error_code& ec) noexcept;
	void rollback_impl(std::error_code& ec) noexcept;

public:
	explicit refTransaction(sqlite::connection_ref conn) noexcept : m_conn(conn) {}

	sqlite::connection_ref GetConnection() noexcept { return m_conn; }
	void Close();

	[[nodiscard]] cTask<> Begin(cTransactionType type, std::error_code& ec);
	[[nodiscard]] cTask<> Commit(std::error_code& ec);
	[[nodiscard]] cTask<> Rollback(std::error_code& ec);

	[[nodiscard]] cTask<> Begin(cTransactionType type = cTransactionType::Deferred);
	[[nodiscard]] cTask<> Commit();
	[[nodiscard]] cTask<> Rollback();
};

/**
 * A RAII type owning an sqlite connection that auto-rollbacks the underlying transaction unless explicitly committed
 */
class cTransaction : public refTransaction {
public:
	/* Become the owner of a database connection since only one transaction is allowed per connection */
	explicit cTransaction(sqlite::connection conn) noexcept : refTransaction(conn.release()) {}

	cTransaction(const cTransaction&) = delete;
	cTransaction(cTransaction&& o) noexcept : refTransaction(std::exchange(o.m_conn, {})) {}

	~cTransaction() { Close(); }

	cTransaction& operator=(cTransaction o) noexcept {
		std::swap<refTransaction>(*this, o);
		return *this;
	}

	[[nodiscard]]
	static cTask<cTransaction> New();

	sqlite::connection ReleaseConnection(std::error_code& ec) noexcept;
	sqlite::connection ReleaseConnection();
};

/**
 * A base type for all DAO classes
 */
class cBaseDAO {
	[[nodiscard]]
	static cTask<> resume_on_db_strand();
	[[nodiscard]]
	static cTask<> wait_on_db_strand(std::chrono::milliseconds);

protected:
	sqlite::connection_ref m_conn;

	explicit cBaseDAO(refTransaction txn) noexcept : m_conn(txn.GetConnection()) {}

	/**
	 * A helper function that invokes a given function on the database strand.
	 * If SQLITE_BUSY errors occur, most likely due on an active transaction, multiple attempts are made to invoke it again, with a timeout of about 30 seconds.
	 * @param f The function or callable object to be invoked in the database strand
	 * @return An awaitable task returning f's result
	 */
	template<std::invocable F>
	[[nodiscard]] cTask<std::invoke_result_t<F>> Exec(F f) {
		/* Make sure we're running on the database strand */
		co_await resume_on_db_strand();

		for (int i = 0;;) {
			try {
				co_return std::invoke(f);
			} catch (const std::system_error& ex) {
				/* If the timeout expired or the error is not SQLITE_BUSY, allow the exception to propagate up */
				if (ex.code() != sqlite::error::busy || ++i >= 300)
					throw;
			}
			/* Rollback the current transaction, if there is one... */
			co_await refTransaction(m_conn).Rollback();
			/* ...and wait a bit before retrying */
			co_await wait_on_db_strand(std::chrono::milliseconds(100));
		}
	}
};
#endif /* GREEKBOT_DATABASE_H */