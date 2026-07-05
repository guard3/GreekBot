#include "CompromisedAccounts.h"
#include "ComproQueries.h"
#include "Message.h"

using namespace std::chrono;

cTask<compromised_entry>
cCompromisedAccountsDAO::GetMetrics(const cMessage& msg) {
	return Exec([=, this] {
		auto[stmt, _] = m_conn.prepare(QUERY_COMPRO_METRICS);
		stmt.bind(1, msg.GetAuthor().GetId());
		if (auto content = msg.GetContent(); content.empty())
			stmt.bind(2, nullptr);
		else
			stmt.bind(2, content);
		stmt.bind(3, msg.GetId());

		return stmt.step() ? compromised_entry{ stmt.column_int(0), milliseconds(stmt.column_int(1)) } : compromised_entry{};
	});
}
