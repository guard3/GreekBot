#include "Member.h"
#include "Welcoming.h"
#include "WelcomingQueries.h"

cTask<uint64_t>
cWelcomingDAO::RegisterMember(const cMember& member) {
	return Exec([&] {
		auto[stmt, _] = m_conn.prepare(QUERY_WC_REGISTER_MEMBER);
		stmt.bind(1, member.GetUser()->GetId());
		stmt.bind(2, member.JoinedAt().time_since_epoch().count());
		/* Make sure the statement returns at least one row */
		if (!stmt.step())
			throw std::system_error(sqlite::error::internal);
		return static_cast<uint64_t>(stmt.column_int(0));
	});
}

cTask<>
cWelcomingDAO::UpdateMessage(crefUser user, crefMessage msg) {
	return Exec([=, this] {
		auto[stmt, _] = m_conn.prepare(QUERY_WC_UPDATE_MESSAGE);
		stmt.bind(1, user.GetId());
		stmt.bind(2, msg.GetId());

		stmt.step();
	});
}

cTask<int64_t>
cWelcomingDAO::GetMessage(const cMemberUpdate& member) {
	return Exec([&] {
		auto[stmt, _] = m_conn.prepare(QUERY_WC_GET_MESSAGE);
		stmt.bind(1, member.GetUser().GetId());

		return stmt.step() ? stmt.column_int(0) : -1;
	});
}

cTask<>
cWelcomingDAO::EditMessage(int64_t msg_id) {
	return Exec([=, this] {
		auto[stmt, _] = m_conn.prepare(QUERY_WC_EDIT_MESSAGE);
		stmt.bind(1, msg_id);
		stmt.step();
	});
}

cTask<uint64_t>
cWelcomingDAO::DeleteMember(crefUser user) {
	return Exec([=, this] {
		auto[stmt, _] = m_conn.prepare(QUERY_WC_DELETE_MEMBER);
		stmt.bind(1, user.GetId());

		return stmt.step() ? static_cast<uint64_t>(stmt.column_int(0)) : 0;
	});
}
