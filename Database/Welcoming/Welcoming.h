#ifndef GREEKBOT_WELCOMING_H
#define GREEKBOT_WELCOMING_H
#include "Database.h"
#include "MemberFwd.h"
#include "MessageFwd.h"
#include "UserFwd.h"

struct cWelcomingDAO : cBaseDAO {
	explicit cWelcomingDAO(refTransaction txn) noexcept : cBaseDAO(txn) {}

	[[nodiscard]] cTask<uint64_t> RegisterMember(const cMember&);
	[[nodiscard]] cTask<> UpdateMessage(crefUser, crefMessage);
	[[nodiscard]] cTask<int64_t> GetMessage(const cMemberUpdate&);
	[[nodiscard]] cTask<> EditMessage(int64_t);
	[[nodiscard]] cTask<uint64_t> DeleteMember(crefUser);
};
#endif //GREEKBOT_WELCOMING_H