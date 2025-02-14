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
cInfractionsDAO::GetEntriesByUser(crefUser user, std::chrono::sys_time<std::chrono::milliseconds> before) {
	using namespace std::chrono;
	auto[stmt, _] = m_conn.prepare(QUERY_WARN_GET);
	stmt.bind(1, user.GetId());
	stmt.bind(2, before.time_since_epoch().count());
	std::vector<infraction_entry> result;
	while (stmt.step()) {
		auto& entry = result.emplace_back(sys_time(milliseconds(stmt.column_int(0))));
		if (auto str = stmt.column_text(1))
			entry.reason = str;
	}
	return result;
}

infraction_result
cInfractionsDAO::GetStatsByUser(crefUser user, std::chrono::sys_time<std::chrono::milliseconds> now) {
	using namespace std::chrono;
	auto[stmt, _] = m_conn.prepare(QUERY_WARN_STATS);
	stmt.bind(1, user.GetId());
	stmt.bind(2, now.time_since_epoch().count());
	stmt.bind(3, floor<milliseconds>(days(1)).count());
	stmt.bind(4, floor<milliseconds>(weeks(1)).count());

	infraction_result result;
	if (stmt.step()) {
		result.stats_today = stmt.column_int(0);
		if (stmt.step()) {
			result.stats_this_week = stmt.column_int(0);
			if (stmt.step()) {
				result.stats_total = stmt.column_int(0);
				result.entries.reserve(10);
				while (stmt.step()) {
					auto& entry = result.entries.emplace_back(sys_time(milliseconds(stmt.column_int(0))));
					if (const char* str = stmt.column_text(1))
						entry.reason = str;
				}
				return result;
			}
		}
	}
	throw std::system_error(SQLITE_INTERNAL, sqlite::error_category());
}

void
cInfractionsDAO::Delete(crefUser user, std::chrono::sys_time<std::chrono::milliseconds> timestamp) {
	auto[stmt, _] = m_conn.prepare(QUERY_WARN_DELETE);
	stmt.bind(1, user.GetId());
	stmt.bind(2, timestamp.time_since_epoch().count());
	while (stmt.step());
}

void
cInfractionsDAO::DeleteAll(crefUser user) {
	auto[stmt, _] = m_conn.prepare(QUERY_WARN_DELETE_ALL);
	stmt.bind(1, user.GetId());
	while (stmt.step());
}
