#include "MessageLog.h"
#include "MessageLogQueries.h"

void
cMessageLogDAO::Register(const cMessage& msg) {
	auto[stmt, _] = m_conn.prepare(QUERY_MSGLOG_REGISTER);
	stmt.bind(1, msg.GetId());
	stmt.bind(2, msg.GetChannelId());
	stmt.bind(3, msg.GetAuthor().GetId());
	auto content = msg.GetContent();
	content.empty() ? stmt.bind(4, nullptr) : stmt.bind(4, content);
	while (stmt.step());
}

std::optional<message_entry>
cMessageLogDAO::Get(crefMessage msg) {
	auto[stmt, _] = m_conn.prepare(QUERY_MSGLOG_GET);
	stmt.bind(1, msg.GetId());
	std::optional<message_entry> result;
	if (stmt.step()) {
		message_entry& entry = result.emplace(
			msg.GetId(),
			stmt.column_int(0),
			stmt.column_int(1)
		);
		if (const char* text = stmt.column_text(2))
			entry.content = text;
	}
	return result;
}

void
cMessageLogDAO::Update(crefMessage msg, std::string_view content) {
	auto[stmt, _] = m_conn.prepare(QUERY_MSGLOG_UPDATE);
	content.empty() ? stmt.bind(1, nullptr) : stmt.bind(1, content);
	stmt.bind(2, msg.GetId());
	while (stmt.step());
}

std::vector<message_entry>
cMessageLogDAO::Delete(std::span<const cSnowflake> ids) {
	std::vector<message_entry> result;
	if (!ids.empty()) {
		/* Prepare statement */
		auto[stmt, _] = m_conn.prepare([&] {
			std::string query = "DELETE FROM messages WHERE id IN (?";
			for (std::size_t i = 1; i < ids.size(); ++i)
				query += ",?";
			return query += ") RETURNING *;";
		}());
		for (int i = 0; i < ids.size(); ++i)
			stmt.bind(i + 1, ids[i]);
		/* Retrieve result */
		for (result.reserve(ids.size()); stmt.step();) {
			message_entry& entry = result.emplace_back(
				stmt.column_int(0),
				stmt.column_int(1),
				stmt.column_int(2)
			);
			if (const char* text = stmt.column_text(3))
				entry.content = text;
		}
	}
	return result;
}

void
cMessageLogDAO::Cleanup() {
	using namespace std::chrono;
	using namespace std::chrono_literals;
	auto[stmt, _] = m_conn.prepare(QUERY_MSGLOG_CLEANUP);
	stmt.bind(1, duration_cast<milliseconds>(system_clock::now() - sys_days(2015y/1/1) - 15 * 24h).count());
	stmt.step();
}