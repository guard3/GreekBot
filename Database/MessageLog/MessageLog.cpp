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
