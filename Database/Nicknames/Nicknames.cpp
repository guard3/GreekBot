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

cTask<std::optional<cSnowflake>>
cNicknamesDAO::Update(crefUser user, std::string_view nick) {
	return Exec([=, this] {
		auto [stmt, _] = m_conn.prepare(QUERY_NICK_UPDATE);
		stmt.bind(1, nick);
		stmt.bind(2, user.GetId());

		std::optional<cSnowflake> result;
		if (std::int64_t value; stmt.step() && (value = stmt.column_int(0)))
			result.emplace(value);
		return result;
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

cTask<nickname_entry>
cNicknamesDAO::GetEntry(crefUser user) {
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

cTask<>
cNicknamesDAO::RegisterMessage(crefUser user, crefMessage msg) {
	return Exec([=, this] {
		auto [stmt, _] = m_conn.prepare(QUERY_NICK_REGISTER_MESSAGE);
		stmt.bind(1, user.GetId());
		stmt.bind(2, msg.GetId());

		while (stmt.step());
	});
}