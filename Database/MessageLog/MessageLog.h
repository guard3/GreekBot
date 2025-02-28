#ifndef GREEKBOT_MESSAGELOG_H
#define GREEKBOT_MESSAGELOG_H
#include "Message.h"
#include "Transaction.h"

struct message_entry {
	cSnowflake id;
	cSnowflake channel_id;
	cSnowflake author_id;
	std::string content;
};

class cMessageLogDAO {
	sqlite::connection_ref m_conn;

public:
	explicit cMessageLogDAO(cTransaction& txn) noexcept : m_conn(txn.GetConnection()) {}

	void Register(const cMessage&);
	std::optional<message_entry> Get(crefMessage msg);
	void Update(crefMessage msg, std::string_view content);
	std::vector<message_entry> Delete(std::span<const cSnowflake>);
	void Cleanup();
};
#endif //GREEKBOT_MESSAGELOG_H
