#pragma once
#ifndef _GREEKBOT_DATABASE_H_
#define _GREEKBOT_DATABASE_H_
#include <sqlite3.h>

class cDatabase final {
private:
	static cDatabase ms_instance;

	static inline sqlite3* ms_pDB;

	cDatabase();
public:

	~cDatabase() { sqlite3_close(ms_pDB); }
};

#endif /* _GREEKBOT_DATABASE_H_ */
