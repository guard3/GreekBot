#pragma once
#ifndef _GREEKBOT_DATABASE_H_
#define _GREEKBOT_DATABASE_H_
#include "Message.h"
#include "Task.h"
#include <vector>
#include <stdexcept>

class sqlite3;
class xDatabaseError : public std::runtime_error {
private:
	int m_code;

public:
	explicit xDatabaseError(sqlite3*);

	int code() const noexcept { return m_code; }
};

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

class cDatabase final {
private:
	static cDatabase ms_instance;

	static sqlite3* ms_pDB;

	cDatabase();

public:
	cDatabase(const cDatabase&) = delete;
	cDatabase(cDatabase&&) = delete;
	~cDatabase();

	cDatabase& operator=(cDatabase) = delete;

	static cTask<> UpdateLeaderboard(const cMessage&);
	static cTask<tRankQueryData> GetUserRank(const cUser&);
	static cTask<tRankQueryData> GetTop10();
};

#endif /* _GREEKBOT_DATABASE_H_ */
