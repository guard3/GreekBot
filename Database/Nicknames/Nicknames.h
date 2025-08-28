#ifndef GREEKBOT_NICKNAMES_H
#define GREEKBOT_NICKNAMES_H
#include "Database.h"
#include "Member.h"
#include "MessageFwd.h"
#include <optional>

struct nickname_entry {
	std::optional<cSnowflake> msg_id;
	std::string nick;
};

class cNicknamesDAO : public cBaseDAO {
public:
	explicit cNicknamesDAO(refTransaction txn) noexcept : cBaseDAO(txn) {}

	[[nodiscard]]
	cTask<std::optional<cSnowflake>> DeleteMessage(crefUser);
	[[nodiscard]]
	cTask<> Update(crefUser, std::string_view);
	[[nodiscard]]
	cTask<std::optional<cSnowflake>> GetMessage(crefUser);
	[[nodiscard]]
	cTask<nickname_entry> GetEntry(crefUser);
	[[nodiscard]]
	cTask<> RegisterMessage(crefUser, crefMessage);
};

#endif //GREEKBOT_NICKNAMES_H
