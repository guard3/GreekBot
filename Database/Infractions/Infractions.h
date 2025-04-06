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

class cInfractionsDAO : cBaseDAO {
public:
	using sys_milliseconds = std::chrono::sys_time<std::chrono::milliseconds>;

	explicit cInfractionsDAO(cTransaction& txn) noexcept : cBaseDAO(txn) {}

	/* Given user, timepoint and reason, register a new infraction and return the delta between the 2 most recent infractions */
	[[nodiscard]] cTask<> Register(crefUser user, sys_milliseconds timepoint, std::string_view reason);
	[[nodiscard]] cTask<std::chrono::milliseconds> GetRecentDeltaTime(crefUser user);
	[[nodiscard]] cTask<infraction_result> GetStatsByUser(crefUser user, sys_milliseconds now);
	[[nodiscard]] cTask<> Delete(crefUser user, sys_milliseconds timestamp);
	[[nodiscard]] cTask<> DeleteAll(crefUser user);
	[[nodiscard]] cTask<> TimeOut(crefUser user, sys_milliseconds now);
};
#endif /* GREEKBOT_INFRACTIONS_H */