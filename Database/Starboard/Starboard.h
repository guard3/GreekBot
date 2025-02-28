#ifndef GREEKBOT_STARBOARD_H
#define GREEKBOT_STARBOARD_H
#include "Transaction.h"

class cStarboardDAO {
	sqlite::connection_ref m_conn;

public:
	explicit cStarboardDAO(cTransaction& txn) noexcept : m_conn(txn.GetConnection()) {}

	std::vector<cSnowflake> DeleteAll(std::span<const cSnowflake> msg_ids);
};
#endif //GREEKBOT_STARBOARD_H