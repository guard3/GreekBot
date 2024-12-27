#ifndef GREEKBOT_SQLITE_H
#define GREEKBOT_SQLITE_H
#include <filesystem>
#include <system_error>
#include <type_traits>
#include <utility>
#include <sqlite3.h>

namespace sqlite {
	const std::error_category& error_category() noexcept;

	class statement;
	template<typename> struct prepare_result;

	/* A non owning reference of a database connection */
	class connection_ref {
		sqlite3* m_pConn;

	public:
		connection_ref(sqlite3* db = nullptr) noexcept : m_pConn(db) {}
		operator sqlite3*(this connection_ref self) noexcept { return self.m_pConn; }

		void close() noexcept {
			sqlite3_close_v2(m_pConn);
			m_pConn = nullptr;
		}

		const char* errmsg(this connection_ref self) noexcept { return sqlite3_errmsg(self); }

		prepare_result<char> prepare(this connection_ref, std::string_view sql, std::error_code& ec) noexcept;
		prepare_result<char> prepare(this connection_ref, std::string_view sql);

		prepare_result<char8_t> prepare(this connection_ref, std::u8string_view sql, std::error_code& ec) noexcept;
		prepare_result<char8_t> prepare(this connection_ref, std::u8string_view sql);
	};

	[[nodiscard]] connection_ref open(const char*, int, std::error_code&) noexcept;
	[[nodiscard]] connection_ref open(const char*, int);
	[[nodiscard]] connection_ref open(const std::filesystem::path&, int, std::error_code&);
	[[nodiscard]] connection_ref open(const std::filesystem::path&, int);

	/* This is a customization point to allow for custom types to be bound to a prepared statement */
	template<typename>
	struct binder;
	/* A non owning reference of a prepared statement */
	class statement_ref {
		sqlite3_stmt* m_pStmt;

	public:
		statement_ref(sqlite3_stmt* stmt = nullptr) noexcept : m_pStmt(stmt) {}

		operator sqlite3_stmt*(this statement_ref self) noexcept { return self.m_pStmt; }

		void finalize() noexcept {
			sqlite3_finalize(m_pStmt);
			m_pStmt = nullptr;
		}

		connection_ref db_handle(this statement_ref self) noexcept {
			return sqlite3_db_handle(self.m_pStmt);
		}

		/* A function to bind a value to a prepared statement */
		template<typename T>
		void bind(this statement_ref self, int index, T&& value, std::error_code& ec) {
			binder<std::remove_cvref_t<T>>::bind(self, index, std::forward<T>(value), ec);
		}
		template<typename T>
		void bind(this statement_ref self, int index, T&& value) {
			std::error_code ec;
			if (self.bind(index, std::forward<T>(value), ec); ec)
				throw std::system_error(ec, self.db_handle().errmsg());
		}

		bool step(this statement_ref, std::error_code& ec) noexcept;
		bool step(this statement_ref);
	};
	/* Predefine some defaults for ints and strings */
	struct binder_base {
		static void bind_impl(sqlite3_stmt*, int, sqlite3_int64, std::error_code&) noexcept;
	};
	/* Specialize for integers */
	template<std::integral T>
	struct binder<T> : binder_base {
		static void bind(statement_ref stmt, int index, T value, std::error_code& ec) noexcept {
			bind_impl(stmt, index, static_cast<sqlite3_int64>(value), ec);
		}
	};

	class connection : public connection_ref {
	public:
		explicit connection(connection_ref ref = {}) noexcept : connection_ref(ref) {}

		connection(const char* filename, int flags, std::error_code& ec) noexcept : connection_ref(sqlite::open(filename, flags, ec)) {}
		connection(const char* filename, int flags) : connection_ref(sqlite::open(filename, flags)) {}
		connection(const std::filesystem::path& filename, int flags, std::error_code& ec) : connection_ref(sqlite::open(filename, flags, ec)) {}
		connection(const std::filesystem::path& filename, int flags) : connection_ref(sqlite::open(filename, flags)) {}

		connection(const connection&) = delete;
		connection(connection&& o) noexcept : connection_ref(std::exchange(static_cast<connection_ref&>(o), {})) {}

		~connection() { close(); }

		connection& operator=(connection o) noexcept {
			std::swap(static_cast<connection_ref&>(*this), static_cast<connection_ref&>(o));
			return *this;
		}

		template<typename Path, typename... Args>
		void open(const Path& filename, int flags, Args&&... args) noexcept(std::is_nothrow_constructible_v<connection, Path&&, int, Args&&...>) {
			*this = connection(filename, flags, std::forward<Args>(args)...);
		}
	};

	class statement : public statement_ref {
	public:
		explicit statement(statement_ref ref = {}) noexcept : statement_ref(ref) {}
		statement(const statement&) = delete;
		statement(statement&& o) noexcept : statement_ref(std::exchange(static_cast<statement_ref&>(o), {})) {}
		~statement() { finalize(); }

		statement& operator=(statement o) noexcept {
			std::swap(static_cast<statement_ref&>(*this), static_cast<statement_ref&>(o));
			return *this;
		}
	};

	template<typename CharT>
	struct prepare_result {
		statement stmt;
		std::basic_string_view<CharT> tail;
	};
}
#endif //GREEKBOT_SQLITE_H