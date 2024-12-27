#include "SQLite.h"

namespace sqlite {
	/* Implement SQLite error category */
	const std::error_category& error_category() noexcept {
		struct sqlite_category : std::error_category {
			const char* name() const noexcept override { return "sqlite"; }
			std::string message(int ev) const override { return sqlite3_errstr(ev); }

			std::error_condition default_error_condition(int val) const noexcept override {
				switch (val) {
					case SQLITE_OK:
						return {};
					case SQLITE_NOMEM:
						return std::errc::not_enough_memory;
					default:
						return { val, *this };
				}
			}
		};

		static const sqlite_category cat;
		return cat;
	}

	/* Implement free open() function */
	connection_ref open(const char* filename, int flags, std::error_code& ec) noexcept {
		sqlite3* db{};

		if (int rc = sqlite3_open_v2(filename, &db, flags, nullptr); !db) {
			ec.assign(SQLITE_NOMEM, error_category());
		} else if (rc == SQLITE_OK) {
			ec.clear();
		} else {
			int sys_err = sqlite3_system_errno(db);
			sqlite3_close(db);
			db = nullptr;
			sys_err ? ec.assign(sys_err, std::system_category()) : ec.assign(rc, error_category());
		}

		return db;
	}
	connection_ref open(const char* filename, int flags) {
		std::error_code ec;
		auto db = open(filename, flags, ec);
		if (ec)
			throw std::system_error(ec);
		return db;
	}
	connection_ref open(const std::filesystem::path& filename, int flags, std::error_code& ec) {
		return open(reinterpret_cast<const char*>(filename.u8string().c_str()), flags, ec);
	}
	connection_ref open(const std::filesystem::path& filename, int flags) {
		return open(reinterpret_cast<const char*>(filename.u8string().c_str()), flags);
	}

	template<typename CharT>
	static prepare_result<CharT> prepare_impl(sqlite3* db, std::basic_string_view<CharT> sql, std::error_code& ec) noexcept {
		sqlite3_stmt* stmt{};
		const char* tail{};
		const char* str = reinterpret_cast<const char*>(sql.data());
		if (sql.size() > std::numeric_limits<int>::max()) {
			ec = make_error_code(std::errc::invalid_argument);
		} else if (int rc = sqlite3_prepare_v2(db, str, static_cast<int>(sql.size()), &stmt, &tail); rc == SQLITE_OK) {
			ec.clear();
			return { statement(stmt), sql.substr(tail - str) };
		} else {
			ec.assign(rc, error_category());
		}

		return { statement(stmt) };
	}
	template<typename CharT>
	static prepare_result<CharT> prepare_impl(sqlite3* db, std::basic_string_view<CharT> sql) {
		std::error_code ec;
		if (auto ret = prepare_impl(db, sql, ec); !ec)
			return ret;
		throw std::system_error(ec, sqlite3_errmsg(db));
	}

	prepare_result<char> connection_ref::prepare(this connection_ref self, std::string_view sql, std::error_code& ec) noexcept {
		return prepare_impl(self, sql, ec);
	}
	prepare_result<char8_t> connection_ref::prepare(this connection_ref self, std::u8string_view sql, std::error_code& ec) noexcept {
		return prepare_impl(self, sql, ec);
	}
	prepare_result<char> connection_ref::prepare(this connection_ref self, std::string_view sql) {
		return prepare_impl(self, sql);
	}
	prepare_result<char8_t> connection_ref::prepare(this connection_ref self, std::u8string_view sql) {
		return prepare_impl(self, sql);
	}

	void binder<std::nullptr_t>::bind(sqlite::statement_ref stmt, int index, std::nullptr_t, std::error_code& ec) noexcept {
		int rc = sqlite3_bind_null(stmt, index);
		rc == SQLITE_OK ? ec.clear() : ec.assign(rc, error_category());
	}
	void binder_base::bind_impl(sqlite3_stmt* stmt, int index, sqlite3_int64 value, std::error_code& ec) noexcept {
		int rc = sqlite3_bind_int64(stmt, index, value);
		rc == SQLITE_OK ? ec.clear() : ec.assign(rc, error_category());
	}
	void binder_base::bind_impl(sqlite3_stmt* stmt, int index, std::string_view value, std::error_code& ec) noexcept {
		int rc = sqlite3_bind_text64(stmt, index, value.data(), value.size(), SQLITE_STATIC, SQLITE_UTF8);
		rc == SQLITE_OK ? ec.clear() : ec.assign(rc, error_category());
	}

	bool statement_ref::step(this statement_ref self, std::error_code& ec) noexcept {
		int rc = sqlite3_step(self.m_pStmt);
		bool ret = rc == SQLITE_ROW;
		ret || rc == SQLITE_DONE ? ec.clear() : ec.assign(rc, error_category());
		return ret;
	}
	bool statement_ref::step(this statement_ref self) {
		std::error_code ec;
		if (bool ret = self.step(ec); !ec)
			return ret;
		throw std::system_error(ec, self.db_handle().errmsg());
	}
}