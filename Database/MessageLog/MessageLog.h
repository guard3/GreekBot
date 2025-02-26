#ifndef GREEKBOT_MESSAGELOG_H
#define GREEKBOT_MESSAGELOG_H
#include "Transaction.h"

class cMessageLogDAO {
	sqlite::connection_ref m_conn;

public:
	explicit cMessageLogDAO(cTransaction& txn) noexcept : m_conn(txn.GetConnection()) {}

	void Register(const cMessage&);
};
#endif //GREEKBOT_MESSAGELOG_H
