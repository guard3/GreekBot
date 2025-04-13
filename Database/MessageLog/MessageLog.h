#ifndef GREEKBOT_MESSAGELOG_H
#define GREEKBOT_MESSAGELOG_H
#include "Message.h"
#include "Database.h"

struct message_entry {
	cSnowflake id;
	cSnowflake channel_id;
	cSnowflake author_id;
	std::string content;
};

class cMessageLogDAO : cBaseDAO {
public:
	explicit cMessageLogDAO(refTransaction txn) noexcept : cBaseDAO(txn) {}

	[[nodiscard]] cTask<> Register(const cMessage&);
	[[nodiscard]] cTask<std::optional<message_entry>> Get(crefMessage msg);
	[[nodiscard]] cTask<> Update(crefMessage msg, std::string_view content);
	[[nodiscard]] cTask<std::vector<message_entry>> Delete(std::span<const cSnowflake>);
	[[nodiscard]] cTask<> Cleanup();
};
#endif //GREEKBOT_MESSAGELOG_H
