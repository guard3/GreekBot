#ifndef GREEKBOT_COMPROMISEDACCOUNTS_H
#define GREEKBOT_COMPROMISEDACCOUNTS_H
#include "Database.h"
#include "MessageFwd.h"

struct compromised_entry {
	std::int64_t num_channels;
	std::chrono::milliseconds period;
};

struct cCompromisedAccountsDAO : cBaseDAO {
	explicit cCompromisedAccountsDAO(refTransaction txn) : cBaseDAO(txn) {}

	[[nodiscard]] cTask<compromised_entry> GetMetrics(const cMessage& msg);
};

#endif //GREEKBOT_COMPROMISEDACCOUNTS_H