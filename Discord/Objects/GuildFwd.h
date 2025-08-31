#ifndef DISCORD_GUILDFWD_H
#define DISCORD_GUILDFWD_H
#include "BaseFwd.h"
DISCORD_FWDDECL_CLASS(GuildCreate);
DISCORD_FWDDECL_CLASS(Guild);

struct crefGuild : crefBase {
	using crefBase::crefBase;
};
#endif /* DISCORD_GUILDFWD_H */