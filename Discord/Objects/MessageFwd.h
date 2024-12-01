#ifndef DISCORD_MESSAGEFWD_H
#define DISCORD_MESSAGEFWD_H
#include "BaseFwd.h"
/* ================================================================================================================== */
DISCORD_FWDDECL_CLASS(MessageUpdate);
DISCORD_FWDDECL_CLASS(MessageBase);
DISCORD_FWDDECL_CLASS(PartialMessage);
DISCORD_FWDDECL_CLASS(Message);
/* ========== A simple const reference wrapper around either a message or a snowflake =============================== */
class crefMessage final {
private:
	const cSnowflake& m_id;
public:
	crefMessage(const cMessage&) noexcept;
	crefMessage(const cSnowflake& id) noexcept : m_id(id) {}

	const cSnowflake& GetId() const noexcept { return m_id; }
};
#endif /* DISCORD_MESSAGEFWD_H */