#ifndef GREEKBOT_NICKNAMES_H
#define GREEKBOT_NICKNAMES_H
#include "Database.h"
#include "Member.h"
#include <optional>

class cNicknamesDAO : public cBaseDAO {
public:
	explicit cNicknamesDAO(refTransaction txn) noexcept : cBaseDAO(txn) {}

	[[nodiscard]]
	cTask<std::optional<cSnowflake>> Register(const cMember&);
};

#endif //GREEKBOT_NICKNAMES_H
