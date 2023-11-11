#ifndef GREEKBOT_DATABASE_H
#define GREEKBOT_DATABASE_H
#include "Coroutines.h"
#include "Message.h"
#include <stdexcept>
#include <vector>
/* ================================================================================================================== */
class xDatabaseError : public std::runtime_error {
private:
	int m_code;
public:
	xDatabaseError();
	int code() const noexcept { return m_code; }
};
/* ================================================================================================================== */
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

struct starboard_entry {
	cSnowflake author_id;
	int64_t num_msg;
	int64_t react_total;
	int64_t max_react_per_msg;
	int64_t rank;

	starboard_entry(int64_t a, int64_t b, int64_t c, int64_t d, int64_t e): author_id(a), num_msg(b), react_total(c), max_react_per_msg(d), rank(e) {}
};
/* ================================================================================================================== */
class cDatabase final {
private:
	static cDatabase ms_instance;
	cDatabase();
public:
	cDatabase(const cDatabase&) = delete;
	~cDatabase();

	cDatabase& operator=(const cDatabase&) = delete;

	static cTask<> UpdateLeaderboard(const cMessage&);
	static cTask<tRankQueryData> GetUserRank(const cUser&);
	static cTask<tRankQueryData> GetTop10();

	static cTask<uint64_t> WC_RegisterMember(const cMember&);
	static cTask<> WC_UpdateMessage(const cUser&, const cMessage&);
	static cTask<int64_t> WC_GetMessage(const cMemberUpdate&);
	static cTask<> WC_EditMessage(int64_t);
	static cTask<uint64_t> WC_DeleteMember(const cUser&);

	static cTask<int64_t> SB_GetMessageAuthor(const cSnowflake&);
	static cTask<std::pair<int64_t, int64_t>> SB_RegisterReaction(const cSnowflake&, const cSnowflake&);
	static cTask<> SB_RegisterMessage(const cSnowflake&, const cSnowflake&);
	static cTask<std::pair<int64_t, int64_t>> SB_RemoveReaction(const cSnowflake&);
	static cTask<> SB_RemoveMessage(const cSnowflake&);
	static cTask<int64_t> SB_RemoveAll(const cSnowflake&);
	static cTask<std::vector<starboard_entry>> SB_GetTop10(int);
	static cTask<std::vector<starboard_entry>> SB_GetRank(const cUser&, int);
};
#endif /* GREEKBOT_DATABASE_H */