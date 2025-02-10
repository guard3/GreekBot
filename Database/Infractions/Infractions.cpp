#include "Infractions.h"
#include "InfractionsQueries.h"

std::int64_t
cInfractionsDAO::Register(crefUser user, std::chrono::sys_time<std::chrono::milliseconds> timepoint, std::string_view reason) {
	using namespace std::chrono;
	auto[stmt, _] = m_conn.prepare(QUERY_WARN_REGISTER);
	stmt.bind(1, user.GetId());
	stmt.bind(2, timepoint.time_since_epoch().count());
	reason.empty() ? stmt.bind(3, nullptr) : stmt.bind(3, reason);
	stmt.bind(4, floor<milliseconds>(days(15)).count());
	return stmt.step() ? stmt.column_int(0) : 0;
}

std::vector<infraction_entry>
cInfractionsDAO::GetEntriesByUser(crefUser user) {
	using namespace std::chrono;
	auto[stmt, _] = m_conn.prepare(QUERY_WARN_GET);
	stmt.bind(1, user.GetId());
	std::vector<infraction_entry> result;
	while (stmt.step()) {
		auto& entry = result.emplace_back(sys_time(milliseconds(stmt.column_int(0))));
		if (auto str = stmt.column_text(1))
			entry.reason = str;
	}
	return result;
}
