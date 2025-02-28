#include "Starboard.h"

std::vector<cSnowflake>
cStarboardDAO::DeleteAll(std::span<const cSnowflake> msg_ids) {
	std::vector<cSnowflake> result;
	if (!msg_ids.empty()) {
		/* Prepare statement */
		auto[stmt, _] = m_conn.prepare([&] {
			std::string query = "DELETE FROM starboard WHERE msg_id IN (?";
			for (std::size_t i = 1; i < msg_ids.size(); ++i)
				query += ",?";
			return query += ") RETURNING sb_msg_id;";
		} ());
		for (int i = 0; i < msg_ids.size(); ++i)
			stmt.bind(i + 1, msg_ids[i]);
		/* Retrieve result */
		result.reserve(msg_ids.size());
		while (stmt.step())
			if (auto id = stmt.column_int(0))
				result.emplace_back(id);
	}
	return result;
}