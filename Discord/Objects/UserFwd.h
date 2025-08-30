#ifndef DISCORD_USERFWD_H
#define DISCORD_USERFWD_H
#include "BaseFwd.h"
/* ================================================================================================================== */
DISCORD_FWDDECL_CLASS(User);
/* ========== A simple const reference wrapper around either a user or a snowflake ================================== */
class crefUser {
	const cSnowflake* m_pId;

public:
	constexpr crefUser(const cSnowflake& id) noexcept : m_pId(&id) {}

	constexpr const cSnowflake& GetId() const noexcept { return *m_pId; }
};
#endif /* DISCORD_USERFWD_H */