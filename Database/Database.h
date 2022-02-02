#pragma once
#ifndef _GREEKBOT_DATABASE_H_
#define _GREEKBOT_DATABASE_H_
#include <sqlite3.h>
#include "Message.h"

class cDatabase final {
private:
	static cDatabase ms_instance;

	static inline sqlite3* ms_pDB;

	cDatabase();
public:

	static bool UpdateLeaderboard(chMessage);
	static bool GetUserRank(chUser user, bool& user_exists, int64_t& rank, int64_t& xp, int64_t& num_msg);

	~cDatabase() { sqlite3_close(ms_pDB); }
};

#endif /* _GREEKBOT_DATABASE_H_ */
