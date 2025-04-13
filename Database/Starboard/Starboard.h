#ifndef GREEKBOT_STARBOARD_H
#define GREEKBOT_STARBOARD_H
#include "Database.h"
#include <optional>
/* ================================================================================================================== */
struct reaction_entry {
	std::optional<cSnowflake> starboard_message_id;
	std::int64_t num_reactions;
};
/* ================================================================================================================== */
struct starboard_entry {
	cSnowflake author_id;
	std::int64_t num_msg;
	std::int64_t react_total;
	std::int64_t max_react_per_msg;
	std::int64_t rank;
};
/* ================================================================================================================== */
class cStarboardDAO : cBaseDAO {
public:
	explicit cStarboardDAO(refTransaction txn) noexcept : cBaseDAO(txn) {}

	[[nodiscard]] cTask<std::optional<cSnowflake>> GetMessageAuthor(crefMessage message);
	[[nodiscard]] cTask<reaction_entry> RegisterReaction(crefMessage message, crefUser author);
	[[nodiscard]] cTask<reaction_entry> RemoveReaction(crefMessage message);
	[[nodiscard]] cTask<> RegisterMessage(crefMessage message, crefMessage starboard_message);
	[[nodiscard]] cTask<> RemoveMessage(crefMessage message);

	[[nodiscard]] cTask<std::vector<cSnowflake>> DeleteAll(std::span<const cSnowflake> msg_ids);

	[[nodiscard]] cTask<std::vector<starboard_entry>> GetTop10(int threshold);
	// TODO: use an optional
	[[nodiscard]] cTask<std::vector<starboard_entry>> GetRank(crefUser user, int threshold);
};
#endif //GREEKBOT_STARBOARD_H