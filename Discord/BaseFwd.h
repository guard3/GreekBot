#ifndef DISCORD_BASEFWD_H
#define DISCORD_BASEFWD_H
#include "Base/ColorFwd.h"
#include "Base/SnowflakeFwd.h"

/**
 * A base class provided for easily defining cref class definitions
 */
class crefBase {
	const cSnowflake* m_pId;

public:
	constexpr crefBase(const cSnowflake& id) noexcept : m_pId(&id) {}

	constexpr const cSnowflake& GetId(this auto self) noexcept { return *self.m_pId; }
};
#endif /* DISCORD_BASEFWD_H */