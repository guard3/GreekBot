#ifndef DISCORD_MESSAGEFWD_H
#define DISCORD_MESSAGEFWD_H
#include "Common.h"
/* ================================================================================================================== */
class cMessageParams;
using   hMessageParams =   hHandle<cMessageParams>;
using  chMessageParams =  chHandle<cMessageParams>;
using  uhMessageParams =  uhHandle<cMessageParams>;
using uchMessageParams = uchHandle<cMessageParams>;
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