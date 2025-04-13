#ifndef GREEKBOT_LEADERBOARD_H
#define GREEKBOT_LEADERBOARD_H
#include "Database.h"

struct leaderboard_entry {
	cSnowflake   user_id;
	std::int64_t rank{};
	std::int64_t xp{};
	std::int64_t num_msg{};
};

class cLeaderboardDAO : cBaseDAO {
public:
	explicit cLeaderboardDAO(refTransaction txn) noexcept : cBaseDAO(txn) {}

	/* Update based on a message and return the new XP of the author */
	[[nodiscard]] cTask<std::int64_t> Update(const cMessage&);
	/* Get a leaderboard entry from a user's id */
	[[nodiscard]] cTask<std::optional<leaderboard_entry>> GetEntryByUser(crefUser);
	/* Get leaderboard entries of at least 10 top performing users */
	[[nodiscard]] cTask<std::vector<leaderboard_entry>> GetTop10Entries();
};
#endif /* GREEKBOT_LEADERBOARD_H */