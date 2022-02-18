#include "Database.h"
#include "Queries.h"
#include "Utils.h"
#include <cstdlib>

/* The filename of the database */
#define DB_FILENAME "database.db"

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
	#include <Shlwapi.h>
#else
	#include <climits>
	#include <cstring>
	#ifdef __APPLE__
		#include <mach-o/dyld.h>
	#else
		#include <unistd.h>
	#endif
#endif

cDatabase cDatabase::ms_instance;

cDatabase::cDatabase() {
	/* Get the path of the database file */
	char* db_filename = nullptr;
#ifdef _WIN32
	DWORD dwSize = MAX_PATH;
	for (LPWSTR p; (p = (LPWSTR)realloc(db_filename, dwSize * sizeof(WCHAR))); dwSize <<= 1) {
		db_filename = (char*)p;
		DWORD dwLen = GetModuleFileNameW(NULL, p, dwSize);
		if (dwLen == 0) break;
		if (dwLen < dwSize) {
			int utf8size = WideCharToMultiByte(CP_UTF8, 0, p, -1, NULL, 0, NULL, NULL);
			if (utf8size) {
				if (char* q = (char*)malloc(utf8size + sizeof(DB_FILENAME))) {
					free(db_filename);
					db_filename = q;
					WideCharToMultiByte(CP_UTF8, 0, p, -1, db_filename, utf8size, NULL, NULL);
					strcpy(PathFindFileNameA(db_filename), DB_FILENAME);
					goto LABEL_RESOLVED_DBNAME;
				}
			}
		}
	}
#elif defined(__APPLE__)
	uint32_t size = PATH_MAX;
	for (char* p; (p = (char*)realloc(db_filename, size)); db_filename = p) {
		if (0 == _NSGetExecutablePath(p, &size)) {
			(db_filename = realpath(p, nullptr)) ? free(p) : (void)(db_filename = p);
			if ((p = strrchr(db_filename, '/'))) {
				if (ptrdiff_t o = (p - db_filename) + 1; (p = (char*) realloc(db_filename, o + sizeof(DB_FILENAME)))) {
					strcpy((db_filename = p) + o, DB_FILENAME);
					goto LABEL_RESOLVED_DBNAME;
				}
			}
		}
	}
#else
	/* TODO add more link options */
	size_t size = PATH_MAX;
	for (char* p; (p = (char*)realloc(db_filename, size)); size <<= 1) {
		ssize_t len = readlink("/proc/self/exe", db_filename = p, size);
		if (len < 0) break;
		if (len + sizeof(DB_FILENAME) < size) {
			if (!(p = strrchr(db_filename, '/'))) break;
			strcpy(p, DB_FILENAME);
			goto LABEL_RESOLVED_DBNAME;
		}
	}
#endif
	free(db_filename);
	cUtils::PrintErr("Fatal initialisation error.");
	exit(EXIT_FAILURE);

LABEL_RESOLVED_DBNAME:
	/* Open a database handle */
	sqlite3* db;
	int rc = sqlite3_open_v2(db_filename, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, nullptr);
	free(db_filename);
	/* If database handle is open, create leaderboard table */
	if (rc == SQLITE_OK) {
		sqlite3_stmt* stmt;
		if (SQLITE_OK == sqlite3_prepare_v2(db, QUERY_INIT, QRLEN_INIT, &stmt, nullptr)) {
			if (SQLITE_DONE == sqlite3_step(stmt)) {
				sqlite3_finalize(stmt);
				ms_pDB = db;
				cUtils::PrintLog("Database initialized successfully.");
				return;
			}
		}
		sqlite3_finalize(stmt);
	}
	cUtils::PrintErr("Fatal Database Error: %s", sqlite3_errmsg(db));
	sqlite3_close(db);
	exit(EXIT_FAILURE);
}

bool cDatabase::UpdateLeaderboard(chMessage msg) {
	/* Execute QUERY_UPDATE_LB */
	sqlite3_stmt* stmt;
	if (SQLITE_OK == sqlite3_prepare_v2(ms_pDB, QUERY_UPDATE_LB, QRLEN_UPDATE_LB, &stmt, nullptr)) {
		if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, msg->GetAuthor()->GetId()->ToInt())) {
			if (SQLITE_OK == sqlite3_bind_int64(stmt, 2, msg->GetId()->GetTimestamp())) {
				if (SQLITE_DONE == sqlite3_step(stmt)) {
					sqlite3_finalize(stmt);
					return true;
				}
			}
		}
	}
	sqlite3_finalize(stmt);
	cUtils::PrintErr("Database error: %s", sqlite3_errmsg(ms_pDB));
	return false;
}

bool cDatabase::GetUserRank(chUser user, tRankQueryData& result) {
	sqlite3_stmt* stmt;
	if (SQLITE_OK == sqlite3_prepare_v2(ms_pDB, QUERY_GET_RANK, QRLEN_GET_RANK, &stmt, nullptr)) {
		if (SQLITE_OK == sqlite3_bind_int64(stmt, 1, user->GetId()->ToInt())) {
			tRankQueryData res;
			switch (sqlite3_step(stmt)) {
				case SQLITE_ROW:
					/* Statement returned data for the user */
					res.reserve(1);
					res.emplace_back(
						sqlite3_column_int64(stmt, 0),
						*user->GetId(),
						sqlite3_column_int64(stmt, 1),
						sqlite3_column_int64(stmt, 2)
					);
				case SQLITE_DONE:
					/* Statement didn't return, no user data found */
					sqlite3_finalize(stmt);
					result = std::move(res);
					return true;
			}
		}
	}
	sqlite3_finalize(stmt);
	cUtils::PrintErr("Database error: %s", sqlite3_errmsg(ms_pDB));
	return false;
}

bool cDatabase::GetTop10(tRankQueryData& result) {
	int rc;
	tRankQueryData res;
	sqlite3_stmt* stmt;
	if (SQLITE_OK == (rc = sqlite3_prepare_v2(ms_pDB, QUERY_GET_TOP_10, QRLEN_GET_TOP_10, &stmt, nullptr))) {
		res.reserve(10);
		while (SQLITE_ROW == (rc = sqlite3_step(stmt))) {
			res.emplace_back(
				sqlite3_column_int64(stmt, 0),
				sqlite3_column_int64(stmt, 1),
				sqlite3_column_int64(stmt, 2),
				sqlite3_column_int64(stmt, 4)
			);
		}
	}
	sqlite3_finalize(stmt);
	if (rc == SQLITE_DONE) {
		result = std::move(res);
		return true;
	}
	cUtils::PrintErr("Database error: %s", sqlite3_errmsg(ms_pDB));
	return false;
}