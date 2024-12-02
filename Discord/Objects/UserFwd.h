#ifndef DISCORD_USERFWD_H
#define DISCORD_USERFWD_H
#include "BaseFwd.h"
/* ================================================================================================================== */
DISCORD_FWDDECL_CLASS(User);
/* ========== A simple const reference wrapper around either a user or a snowflake ================================== */
class crefUser final {
private:
	const cSnowflake& m_id;
public:
	crefUser(const cUser& user) noexcept;
	crefUser(const cSnowflake& id) noexcept : m_id(id) {}

	const cSnowflake& GetId() const noexcept { return m_id; }
};
#endif /* DISCORD_USERFWD_H */