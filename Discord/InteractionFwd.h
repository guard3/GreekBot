#ifndef DISCORD_INTERACTIONFWD_H
#define DISCORD_INTERACTIONFWD_H
#include "Common.h"
/* ================================================================================================================== */
class   cInteraction;
using   hInteraction =   hHandle<cInteraction>;
using  chInteraction =  chHandle<cInteraction>;
using  uhInteraction =  uhHandle<cInteraction>;
using uchInteraction = uchHandle<cInteraction>;
/* ================================================================================================================== */
class   cAppCmdInteraction;
using   hAppCmdInteraction =   hHandle<cAppCmdInteraction>;
using  chAppCmdInteraction =  chHandle<cAppCmdInteraction>;
using  uhAppCmdInteraction =  uhHandle<cAppCmdInteraction>;
using uchAppCmdInteraction = uchHandle<cAppCmdInteraction>;
/* ================================================================================================================== */
class   cMsgCompInteraction;
using   hMsgCompInteraction =   hHandle<cMsgCompInteraction>;
using  chMsgCompInteraction =  chHandle<cMsgCompInteraction>;
using  uhMsgCompInteraction =  uhHandle<cMsgCompInteraction>;
using uchMsgCompInteraction = uchHandle<cMsgCompInteraction>;
/* ================================================================================================================== */
class   cModalSubmitInteraction;
using   hModalSubmitInteraction =   hHandle<cModalSubmitInteraction>;
using  chModalSubmitInteraction =  chHandle<cModalSubmitInteraction>;
using  uhModalSubmitInteraction =  uhHandle<cModalSubmitInteraction>;
using uchModalSubmitInteraction = uchHandle<cModalSubmitInteraction>;
#endif /* DISCORD_INTERACTIONFWD_H */