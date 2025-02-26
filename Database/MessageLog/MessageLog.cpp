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
		auto& entry = result.emplace(
			msg.GetId(),
			stmt.column_int(0),
			stmt.column_int(1)
		);
		if (auto text = stmt.column_text(2))
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