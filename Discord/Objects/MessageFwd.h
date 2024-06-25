#ifndef DISCORD_MESSAGEFWD_H
#define DISCORD_MESSAGEFWD_H
#include "Common.h"
/* ================================================================================================================== */
class cMessageUpdate;
using   hMessageUpdate =   hHandle<cMessageUpdate>;
using  chMessageUpdate =  chHandle<cMessageUpdate>;
using  uhMessageUpdate =  uhHandle<cMessageUpdate>;
using uchMessageUpdate = uchHandle<cMessageUpdate>;
/* ================================================================================================================== */
class cMessageBase;
using   hMessageBase =   hHandle<cMessageBase>;
using  chMessageBase =  chHandle<cMessageBase>;
using  uhMessageBase =  uhHandle<cMessageBase>;
using uchMessageBase = uchHandle<cMessageBase>;
/* ================================================================================================================== */
class cPartialMessage;
using   hPartialMessage =   hHandle<cPartialMessage>;
using  chPartialMessage =  chHandle<cPartialMessage>;
using  uhPartialMessage =  uhHandle<cPartialMessage>;
using uchPartialMessage = uchHandle<cPartialMessage>;
/* ================================================================================================================== */
class cMessage;
using   hMessage =   hHandle<cMessage>;
using  chMessage =  chHandle<cMessage>;
using  uhMessage =  uhHandle<cMessage>;
using uchMessage = uchHandle<cMessage>;
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