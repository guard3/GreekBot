#ifndef DISCORD_MESSAGEFWD_H
#define DISCORD_MESSAGEFWD_H
#include "BaseFwd.h"
/* ================================================================================================================== */
DISCORD_FWDDECL_CLASS(MessageUpdate);
DISCORD_FWDDECL_CLASS(MessageBase);
DISCORD_FWDDECL_CLASS(PartialMessage);
DISCORD_FWDDECL_CLASS(Message);
/* ========== A simple const reference wrapper around either a message or a snowflake =============================== */
struct crefMessage : crefBase {
	using crefBase::crefBase;
};
#endif /* DISCORD_MESSAGEFWD_H */