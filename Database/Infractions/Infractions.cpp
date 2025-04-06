#include "Infractions.h"
#include "InfractionsQueries.h"

using namespace std::chrono;

cTask<>
cInfractionsDAO::Register(crefUser user, sys_milliseconds timepoint, std::string_view reason) {
	return Exec([=, this] {
		auto[stmt, _] = m_conn.prepare(QUERY_WARN_REGISTER);
		stmt.bind(1, user.GetId());
		stmt.bind(2, timepoint.time_since_epoch().count());
		reason.empty() ? stmt.bind(3, nullptr) : stmt.bind(3, reason);
		while (stmt.step());
	});
}

cTask<std::chrono::milliseconds>
cInfractionsDAO::GetRecentDeltaTime(crefUser user) {
	return Exec([=, this] {
		auto [stmt, _] = m_conn.prepare(QUERY_WARN_DELTA);
		stmt.bind(1, user.GetId());
		return stmt.step() ? milliseconds(stmt.column_int(0)) : milliseconds::max();
	});
}

cTask<infraction_result>
cInfractionsDAO::GetStatsByUser(crefUser user, sys_milliseconds now) {
	return Exec([=, this] {
		using namespace std::chrono;
		auto [stmt, _] = m_conn.prepare(QUERY_WARN_STATS);
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
		throw std::system_error(sqlite::error::internal);
	});
}

cTask<>
cInfractionsDAO::Delete(crefUser user, sys_milliseconds timestamp) {
	return Exec([=, this] {
		auto [stmt, _] = m_conn.prepare(QUERY_WARN_DELETE);
		stmt.bind(1, user.GetId());
		stmt.bind(2, timestamp.time_since_epoch().count());
		while (stmt.step());
	});
}

cTask<>
cInfractionsDAO::DeleteAll(crefUser user) {
	return Exec([=, this] {
		auto [stmt, _] = m_conn.prepare(QUERY_WARN_DELETE_ALL);
		stmt.bind(1, user.GetId());
		while (stmt.step());
	});
}

cTask<>
cInfractionsDAO::TimeOut(crefUser user, sys_milliseconds now) {
	return Exec([=, this] {
		auto [stmt, _] = m_conn.prepare(QUERY_WARN_TIMEOUT);
		stmt.bind(1, user.GetId());
		stmt.bind(2, now.time_since_epoch().count());
		while (stmt.step());
	});
}
