#ifndef GREEKBOT_TEMPBANS_H
#define GREEKBOT_TEMPBANS_H
#include "Database.h"
#include "UserFwd.h"
#include <vector>
#include <span>

class cTempBanDAO : cBaseDAO {
public:
	explicit cTempBanDAO(refTransaction txn) noexcept : cBaseDAO(txn) {}

	[[nodiscard]] cTask<> Register(crefUser user, std::chrono::sys_days expires_at);
	[[nodiscard]] cTask<std::vector<cSnowflake>> GetExpired();
	[[nodiscard]] cTask<> Remove(crefUser user);
	[[nodiscard]] cTask<> Remove(std::span<const cSnowflake> user_ids);
};
#endif //GREEKBOT_TEMPBANS_H