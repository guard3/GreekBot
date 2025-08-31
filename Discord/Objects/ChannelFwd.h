#ifndef DISCORD_CHANNELFWD_H
#define DISCORD_CHANNELFWD_H
#include "BaseFwd.h"
/* ================================================================================================================== */
DISCORD_FWDDECL_CLASS(Channel);
/* ========== A simple const reference wrapper around either a channel or a snowflake =============================== */
struct crefChannel : crefBase {
	using crefBase::crefBase;
};
#endif /* DISCORD_CHANNELFWD_H */