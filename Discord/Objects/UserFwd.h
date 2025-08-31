#ifndef DISCORD_USERFWD_H
#define DISCORD_USERFWD_H
#include "BaseFwd.h"
/* ================================================================================================================== */
DISCORD_FWDDECL_CLASS(User);
/* ========== A simple const reference wrapper around either a user or a snowflake ================================== */
struct crefUser : crefBase {
	using crefBase::crefBase;
};
#endif /* DISCORD_USERFWD_H */