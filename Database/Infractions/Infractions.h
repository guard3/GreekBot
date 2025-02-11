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
	explicit cInfractionsDAO(sqlite::connection_ref conn) noexcept : m_conn(conn) {}

	/* Given user, timepoint and content, register a new infraction and return the number of most recent infractions within timeout_period */
	std::int64_t Register(crefUser user, std::chrono::sys_time<std::chrono::milliseconds> timepoint, std::string_view reason);
	std::vector<infraction_entry> GetEntriesByUser(crefUser user, std::chrono::sys_time<std::chrono::milliseconds> before);
	infraction_result GetStatsByUser(crefUser user, std::chrono::sys_time<std::chrono::milliseconds> now);
	void Delete(std::chrono::sys_time<std::chrono::milliseconds> timestamp);
	void DeleteAll(crefUser);
};
#endif /* GREEKBOT_INFRACTIONS_H */