#ifndef GREEKBOT_INFRACTIONS_H
#define GREEKBOT_INFRACTIONS_H
#include "Database.h"

struct infraction_entry {
	std::chrono::sys_time<std::chrono::milliseconds> timestamp;
	std::string reason;
};

struct infraction_result {
	std::int64_t stats_today{}, stats_this_week{}, stats_total{};
	std::vector<infraction_entry> entries;
};

class cInfractionsDAO {
	sqlite::connection_ref m_conn;

public:
	using sys_milliseconds = std::chrono::sys_time<std::chrono::milliseconds>;

	explicit cInfractionsDAO(cTransaction& txn) noexcept : m_conn(txn.GetConnection()) {}

	/* Given user, timepoint and reason, register a new infraction and return the delta between the 2 most recent infractions */
	void Register(crefUser user, sys_milliseconds timepoint, std::string_view reason);
	std::chrono::milliseconds GetRecentDeltaTime(crefUser user);
	infraction_result GetStatsByUser(crefUser user, sys_milliseconds now);
	void Delete(crefUser user, sys_milliseconds timestamp);
	void DeleteAll(crefUser);
	void TimeOut(crefUser user, sys_milliseconds now);
};
#endif /* GREEKBOT_INFRACTIONS_H */