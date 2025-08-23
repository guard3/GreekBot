#include "Nicknames.h"
#include "NicknamesQueries.h"

cTask<>
cNicknamesDAO::Register(const cMember& member) {
	return Exec([&] {
		auto [stmt, _] = m_conn.prepare(QUERY_NICK_REGISTER);
		stmt.bind(1, member.GetUser()->GetId());

		while (stmt.step());
	});
}

cTask<>
cNicknamesDAO::DeleteMessage(crefUser user) {
	return Exec([=, this] {
		auto [stmt, _] = m_conn.prepare(QUERY_NICK_DELETE_MESSAGE);
		stmt.bind(1, user.GetId());

		while (stmt.step());
	});
}

cTask<>
cNicknamesDAO::Update(crefUser user, std::string_view nick) {
	return Exec([=, this] {
		auto [stmt, _] = m_conn.prepare(QUERY_NICK_UPDATE);
		stmt.bind(1, nick);
		stmt.bind(2, user.GetId());

		while (stmt.step());
	});
}

cTask<std::optional<cSnowflake>>
cNicknamesDAO::GetMessage(crefUser user) {
	return Exec([=, this] {
		auto [stmt, _] = m_conn.prepare(QUERY_NICK_GET_MESSAGE);
		stmt.bind(1, user.GetId());

		std::optional<cSnowflake> result;
		if (std::int64_t value; stmt.step() && (value = stmt.column_int(0)))
			result.emplace(value);

		return result;
	});
}

cTask<>
cNicknamesDAO::RegisterMessage(crefUser user, crefMessage msg) {
	return Exec([=, this] {
		auto [stmt, _] = m_conn.prepare(QUERY_NICK_REGISTER_MESSAGE);
		stmt.bind(1, user.GetId());
		stmt.bind(2, msg.GetId());

		while (stmt.step());
	});
}