#ifndef DISCORD_CHANNELFWD_H
#define DISCORD_CHANNELFWD_H
#include "BaseFwd.h"
/* ================================================================================================================== */
DISCORD_FWDDECL_CLASS(Channel);
/* ========== A simple const reference wrapper around either a channel or a snowflake =============================== */
class crefChannel {
	const cSnowflake* m_pId;

public:
	constexpr crefChannel(const cSnowflake& id) noexcept : m_pId(&id) {}

	constexpr const cSnowflake& GetId() const noexcept { return *m_pId; }
};
#endif /* DISCORD_CHANNELFWD_H */