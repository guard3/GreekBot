#pragma once
#ifndef _GREEKBOT_DATABASE_H_
#define _GREEKBOT_DATABASE_H_
#include <sqlite3.h>
#include "Message.h"
#include <vector>

class cRankQueryDataElement final {
private:
	int64_t rank;
	cSnowflake id;
	int64_t xp;
	int64_t num_msg;

public:
	cRankQueryDataElement(int64_t rank, cSnowflake id, int64_t xp, int64_t num_msg) : rank(rank), id(id), xp(xp), num_msg(num_msg) {};

	int64_t GetRank()        const { return rank;    }
	int64_t GetXp()          const { return xp;      }
	int64_t GetNumMessages() const { return num_msg; }

	chSnowflake GetUserId() const { return &id; }
};
typedef std::vector<cRankQueryDataElement> tRankQueryData;

struct test {
	int64_t rank;
	int64_t id;
	int64_t xp;
	int64_t num_msg;
};

class cDatabase final {
private:
	static cDatabase ms_instance;

	static inline sqlite3* ms_pDB;

	cDatabase();
public:

	static bool UpdateLeaderboard(chMessage);
	static bool GetUserRank(chUser user, tRankQueryData& result);
	static bool GetTop10(tRankQueryData& result);

	~cDatabase() { sqlite3_close(ms_pDB); }
};

#endif /* _GREEKBOT_DATABASE_H_ */
