#include "Nicknames.h"
#include "NicknamesQueries.h"

cTask<nickname_entry>
cNicknamesDAO::Get(crefUser user) {
	return Exec([=, this] {
		auto [stmt, _] = m_conn.prepare(QUERY_NICK_GET);
		stmt.bind(1, user.GetId());

		nickname_entry result;
		if (stmt.step()) {
			if (auto value = stmt.column_int(0))
				result.msg_id.emplace(value);
			if (auto text = stmt.column_text(1))
				result.nick = text;
		}

		return result;
	});
}

cTask<std::optional<cSnowflake>>
cNicknamesDAO::DeleteMessage(crefUser user) {
	return Exec([=, this] {
		auto [stmt, _] = m_conn.prepare(QUERY_NICK_DELETE_MESSAGE);
		stmt.bind(1, user.GetId());

		std::optional<cSnowflake> result;
		if (std::int64_t value; stmt.step() && (value = stmt.column_int(0)))
			result.emplace(value);

		return result;
	});
}

cTask<>
cNicknamesDAO::Update(crefUser user, std::string_view nick) {
	return Exec([=, this] {
		auto [stmt, _] = m_conn.prepare(QUERY_NICK_UPDATE);
		stmt.bind(1, user.GetId());
		stmt.bind(2, nick);

		while (stmt.step());
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

cTask<std::int64_t>
cNicknamesDAO::Cleanup() {
	return Exec([this]{
		auto [stmt, _] = m_conn.prepare(QUERY_NICK_CLEANUP);

		while (stmt.step());

		return sqlite3_changes64(m_conn);
	});
}