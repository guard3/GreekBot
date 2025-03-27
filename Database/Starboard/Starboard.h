#ifndef GREEKBOT_STARBOARD_H
#define GREEKBOT_STARBOARD_H
#include "Transaction.h"
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
class cStarboardDAO {
	sqlite::connection_ref m_conn;

public:
	explicit cStarboardDAO(cTransaction& txn) noexcept : m_conn(txn.GetConnection()) {}

	std::optional<cSnowflake> GetMessageAuthor(crefMessage message);
	reaction_entry RegisterReaction(crefMessage message, crefUser author);
	reaction_entry RemoveReaction(crefMessage message);
	void RegisterMessage(crefMessage message, crefMessage starboard_message);
	void RemoveMessage(crefMessage message);

	std::vector<cSnowflake> DeleteAll(std::span<const cSnowflake> msg_ids);

	std::vector<starboard_entry> GetTop10(int threshold);
	// TODO: use an optional
	std::vector<starboard_entry> GetRank(crefUser user, int threshold);
};
#endif //GREEKBOT_STARBOARD_H