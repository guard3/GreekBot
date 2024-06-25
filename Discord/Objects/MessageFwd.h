#ifndef DISCORD_MESSAGEFWD_H
#define DISCORD_MESSAGEFWD_H
#include "Common.h"
/* ================================================================================================================== */
class cPartialMessage;
using   hPartialMessage =   hHandle<cPartialMessage>;
using  chPartialMessage =  chHandle<cPartialMessage>;
using  uhPartialMessage =  uhHandle<cPartialMessage>;
using uchPartialMessage = uchHandle<cPartialMessage>;
/* ================================================================================================================== */
class cMessageUpdate;
using   hMessageUpdate =   hHandle<cMessageUpdate>;
using  chMessageUpdate =  chHandle<cMessageUpdate>;
using  uhMessageUpdate =  uhHandle<cMessageUpdate>;
using uchMessageUpdate = uchHandle<cMessageUpdate>;
/* ================================================================================================================== */
class cMessage;
using   hMessage =   hHandle<cMessage>;
using  chMessage =  chHandle<cMessage>;
using  uhMessage =  uhHandle<cMessage>;
using uchMessage = uchHandle<cMessage>;
#endif /* DISCORD_MESSAGEFWD_H */