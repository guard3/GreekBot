#include "Nicknames.h"
#include "NicknamesQueries.h"

cTask<std::optional<cSnowflake>>
cNicknamesDAO::Register(const cMember& member) {
	return Exec([&] {
		auto [stmt, _] = m_conn.prepare(QUERY_NICK_REGISTER);
		stmt.bind(1, member.GetUser()->GetId());

		std::optional<cSnowflake> result;
		if (std::int64_t value; stmt.step() && (value = stmt.column_int(0))) {
			result.emplace(value);
		}

		return result;
	});
}
