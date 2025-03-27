#include "Starboard.h"
#include "StarboardQueries.h"

std::optional<cSnowflake>
cStarboardDAO::GetMessageAuthor(crefMessage message) {
	auto[stmt, _] = m_conn.prepare(QUERY_SB_GET_MESSAGE_AUTHOR);
	stmt.bind(1, message.GetId());

	return stmt.step() ? std::make_optional<cSnowflake>(stmt.column_int(0)) : std::nullopt;
}

reaction_entry
cStarboardDAO::RegisterReaction(crefMessage message, crefUser author) {
	/* Prepare statement */
	auto[stmt, _] = m_conn.prepare(QUERY_SB_REGISTER_REACTION);
	stmt.bind(1, message.GetId());
	stmt.bind(2, author.GetId());
	/* Make sure the statement returns at least one row */
	if (!stmt.step())
		throw std::system_error(SQLITE_INTERNAL, sqlite::error_category());
	/* Prepare the result */
	reaction_entry result { std::nullopt, stmt.column_int(1) };
	/* If the starboard message id is not 0, add it to the result */
	if (auto val = stmt.column_int(0))
		result.starboard_message_id.emplace(val);
	return result;
}

reaction_entry
cStarboardDAO::RemoveReaction(crefMessage message) {
	auto[stmt, _] = m_conn.prepare(QUERY_SB_REMOVE_REACTION);
	stmt.bind(1, message.GetId());

	reaction_entry result{};
	if (stmt.step()) {
		result.num_reactions = stmt.column_int(1);
		if (auto val = stmt.column_int(0))
			result.starboard_message_id.emplace(val);
	}
	return result;
}

void
cStarboardDAO::RegisterMessage(crefMessage message, crefMessage starboard_message) {
	auto[stmt, _] = m_conn.prepare(QUERY_SB_REGISTER_MESSAGE);
	stmt.bind(1, starboard_message.GetId());
	stmt.bind(2, message.GetId());
	while (stmt.step());
}

void
cStarboardDAO::RemoveMessage(crefMessage message) {
	auto[stmt, _] = m_conn.prepare(QUERY_SB_REMOVE_MESSAGE);
	stmt.bind(1, message.GetId());
	while (stmt.step());
}

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

std::vector<starboard_entry>
cStarboardDAO::GetTop10(int threshold) {
	auto[stmt, _] = m_conn.prepare(QUERY_SB_GET_TOP_10);
	stmt.bind(1, threshold);
	std::vector<starboard_entry> result;
	result.reserve(10);
	for (int64_t rank = 1; stmt.step(); ++rank) {
		result.emplace_back(
			stmt.column_int(0),
			stmt.column_int(1),
			stmt.column_int(2),
			stmt.column_int(3),
			rank
		);
	}
	return result;
}

std::vector<starboard_entry>
cStarboardDAO::GetRank(crefUser user, int threshold) {
	auto[stmt, _] = m_conn.prepare(QUERY_SB_GET_RANK);
	stmt.bind(1, threshold);
	stmt.bind(2, user.GetId());

	std::vector<starboard_entry> result;
	if (stmt.step()) {
		result.emplace_back(
			stmt.column_int(0),
			stmt.column_int(1),
			stmt.column_int(2),
			stmt.column_int(3),
			stmt.column_int(4)
		);
	}
	return result;
}