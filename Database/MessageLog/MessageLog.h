#ifndef GREEKBOT_MESSAGELOG_H
#define GREEKBOT_MESSAGELOG_H
#include "Transaction.h"

class cMessageLogDAO {
	sqlite::connection_ref m_conn;

public:
	explicit cMessageLogDAO(cTransaction& txn) noexcept : m_conn(txn.GetConnection()) {}

	void Register(const cMessage&);
	std::optional<message_entry> Get(crefMessage msg);
	void Update(crefMessage msg, std::string_view content);

};
#endif //GREEKBOT_MESSAGELOG_H
