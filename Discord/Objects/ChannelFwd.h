#ifndef DISCORD_CHANNELFWD_H
#define DISCORD_CHANNELFWD_H
#include "BaseFwd.h"
/* ================================================================================================================== */
DISCORD_FWDDECL_CLASS(Channel);
/* ========== A simple const reference wrapper around either a channel or a snowflake =============================== */
class crefChannel final {
	const cSnowflake& m_id;
public:
	crefChannel(const cChannel& channel) noexcept;
	crefChannel(const cSnowflake& id) noexcept : m_id(id) {}

	const cSnowflake& GetId() const noexcept { return m_id; }
};
#endif /* DISCORD_CHANNELFWD_H */