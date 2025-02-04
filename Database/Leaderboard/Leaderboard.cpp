#include "Leaderboard.h"
#include "LeaderboardQueries.h"

std::int64_t
cLeaderboardDAO::Update(const cMessage& msg) {
	using namespace std::chrono;
	auto[stmt, _] = m_conn.prepare(QUERY_LB_UPDATE);
	stmt.bind(1, msg.GetAuthor().GetId());
	stmt.bind(2, floor<seconds>(msg.GetTimestamp()).time_since_epoch().count());
	return stmt.step() ? stmt.column_int(0) : 0;
}

std::optional<leaderboard_entry>
cLeaderboardDAO::GetEntryByUser(crefUser user) {
	auto[stmt, _] = m_conn.prepare(QUERY_LB_RANK);
	stmt.bind(1, user.GetId());

	std::optional<leaderboard_entry> result;
	if (stmt.step()) {
		result.emplace(
			user.GetId(),
			stmt.column_int(0),
			stmt.column_int(1),
			stmt.column_int(2)
		);
	}
	return result;
}

std::vector<leaderboard_entry>
cLeaderboardDAO::GetTop10Entries() {
	auto[stmt, _] = m_conn.prepare(QUERY_LB_TOP);
	std::vector<leaderboard_entry> result;
	for (result.reserve(10); stmt.step();) {
		result.emplace_back(
			stmt.column_int(0),
			stmt.column_int(1),
			stmt.column_int(2),
			stmt.column_int(3)
		);
	}
	return result;
}