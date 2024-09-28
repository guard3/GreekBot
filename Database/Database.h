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
struct leaderboard_entry {
	cSnowflake    user_id;
	std::uint64_t rank;
	std::uint64_t xp;
	std::uint64_t num_msg;
};

struct starboard_entry {
	cSnowflake author_id;
	int64_t num_msg;
	int64_t react_total;
	int64_t max_react_per_msg;
	int64_t rank;

	starboard_entry(int64_t a, int64_t b, int64_t c, int64_t d, int64_t e): author_id(a), num_msg(b), react_total(c), max_react_per_msg(d), rank(e) {}
};

struct message_entry {
	cSnowflake id;
	cSnowflake channel_id;
	cSnowflake author_id;
	std::string content;
};
/* ================================================================================================================== */
class cDatabase final {
private:
	static cDatabase ms_instance;
	cDatabase() = default;
public:
	cDatabase(const cDatabase&) = delete;
	~cDatabase();

	cDatabase& operator=(const cDatabase&) = delete;

	static void Initialize();

	static cTask<std::uint64_t> UpdateLeaderboard(const cMessage&);
	static cTask<std::optional<leaderboard_entry>> GetUserRank(const cUser&);
	static cTask<std::vector<leaderboard_entry>> GetTop10();

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

	static cTask<> RegisterMessage(const cMessage&);
	static cTask<std::optional<message_entry>> GetMessage(const cSnowflake&);
	static cTask<std::optional<message_entry>> UpdateMessage(const cSnowflake&, std::string_view);
	static cTask<std::optional<message_entry>> DeleteMessage(const cSnowflake&);
	static cTask<std::vector<message_entry>> DeleteMessages(std::span<const cSnowflake>);
	static cTask<> CleanupMessages();

	static cTask<> RegisterTemporaryBan(crefUser user, std::chrono::sys_days expires_at);
	static cTask<> RemoveTemporaryBan(crefUser user);
};
#endif /* GREEKBOT_DATABASE_H */