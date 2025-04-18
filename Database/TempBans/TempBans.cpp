#include "TempBans.h"
#include "TempBanQueries.h"

cTask<>
cTempBanDAO::Register(crefUser user, std::chrono::sys_days expires_at) {
	if (expires_at == std::chrono::sys_days{})
		return Remove(user);
	return Exec([=, this] {
		auto[stmt, _] = m_conn.prepare(QUERY_TEMPBAN_REGISTER);
		stmt.bind(1, user.GetId());
		stmt.bind(2, expires_at.time_since_epoch().count());
		while (stmt.step());
	});
}

cTask<std::vector<cSnowflake>>
cTempBanDAO::GetExpired() {
	return Exec([this] {
		using namespace std::chrono;
		auto [stmt, _] = m_conn.prepare(QUERY_TEMPBAN_GET_EXPIRED);
		stmt.bind(1, floor<days>(system_clock::now()).time_since_epoch().count());
		std::vector<cSnowflake> result;
		while (stmt.step())
			result.emplace_back(stmt.column_int(0));
		return result;
	});
}

cTask<>
cTempBanDAO::Remove(crefUser user) {
	return Exec([=, this] {
		auto [stmt, _] = m_conn.prepare(QUERY_TEMPBAN_REMOVE);
		stmt.bind(1, user.GetId());
		while (stmt.step());
	});
}

cTask<>
cTempBanDAO::Remove(std::span<const cSnowflake> user_ids) {
	return Exec([=, this] {
		if (user_ids.empty())
			return;

		auto [stmt, _] = m_conn.prepare([&] {
			std::string query = "DELETE FROM tempbans WHERE user_id IN (?";
			for (std::size_t i = 1; i < user_ids.size(); ++i)
				query += ",?";
			return query += ");";
		}());

		for (std::size_t i = 0; i < user_ids.size(); ++i)
			stmt.bind(i + 1, user_ids[i]);
		while (stmt.step());
	});
}