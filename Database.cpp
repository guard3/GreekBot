#include "Database.h"
#include "Utils.h"
#include <cstdlib>
#include <cstring>

/* The filename of the database */
#define __DB_FILENAME "database.db"

#ifdef _WIN32
	#define UNICODE
	// include windows...
	#define DB_FILENAME TEXT(__DB_FILENAME)
	#define SQLITE3_OPEN(filename, ppDB) sqlite3_open16(filename, ppDB)
#else
	#include <climits>
	#define DB_FILENAME __DB_FILENAME
	#define SQLITE3_OPEN(filename, ppDB) sqlite3_open(filename, ppDB)
	#define TCHAR char
	#ifdef __APPLE__
		#include <mach-o/dyld.h>
		typedef uint32_t path_len_t;
	#else
		#include <unistd.h>
		typedef size_t path_len_t;
	#endif
#endif

cDatabase cDatabase::ms_instance;

cDatabase::cDatabase() {
	/* Get the path of the database file */
	TCHAR* db_filename = nullptr;
#ifdef _WIN32
	// Windows tba
#else
	path_len_t size = PATH_MAX;
#ifdef __APPLE__
	for (char* p; (p = (char*)realloc(db_filename, size)); db_filename = p) {
		if (0 == _NSGetExecutablePath(p, &size)) {
			(db_filename = realpath(p, nullptr)) ? free(p) : (void)(db_filename = p);
			if ((p = strrchr(db_filename, '/'))) {
				if (ptrdiff_t o = (p - db_filename) + 1; (p = (char*) realloc(db_filename, o + sizeof(DB_FILENAME)))) {
					strcpy((db_filename = p) + o, DB_FILENAME);
					goto LABEL;
				}
			}
		}
	}
#else
	/* TODO add more link options */
	for (char* p; (p = (char*)realloc(db_filename, size)); size <<= 1) {
		ssize_t len = readlink("/proc/self/exe", db_filename = p, size);
		if (len < 0) break;
		if (len + sizeof(DB_FILENAME) < size) {
			if (!(p = strrchr(db_filename, '/'))) break;
			strcpy(p, DB_FILENAME);
			goto LABEL;
		}
	}
#endif
	free(db_filename);
	cUtils::PrintErr("Fatal initialisation error.");
	exit(EXIT_FAILURE);
#endif

LABEL:
	/* Open a database handle */
	sqlite3* db;
	int rc = SQLITE3_OPEN(db_filename, &db);
	free(db_filename);

	/* If database handle is open, create leaderboard table */
	if (rc == SQLITE_OK) {
		char* err_msg;
		rc = sqlite3_exec(
			db,
			"CREATE TABLE IF NOT EXISTS leaderboard(id INTEGER PRIMARY KEY, num_msg INTEGER NOT NULL, xp INTEGER NOT NULL, timestamp INTEGER NOT NULL);",
			nullptr,
			nullptr,
			&err_msg
		);
		if (rc == SQLITE_OK) {
			ms_pDB = db;
			cUtils::PrintLog("Database initialized successfully.");
			return;
		}
		cUtils::PrintErr("Fatal Database Error: %s", err_msg);
		sqlite3_free(err_msg);
	}
	else cUtils::PrintErr("Fatal Database Error: %s", sqlite3_errmsg(db));
	sqlite3_close(db);
	exit(EXIT_FAILURE);
}