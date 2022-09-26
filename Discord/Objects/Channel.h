#ifndef _GREEKBOT_CHANNEL_H_
#define _GREEKBOT_CHANNEL_H_
#include "Common.h"

enum eChannelType {
	CHANNEL_GUILD_TEXT,
	CHANNEL_DM,
	CHANNEL_GUILD_VOICE,
	CHANNEL_GROUP_DM,
	CHANNEL_GUILD_CATEGORY,
	CHANNEL_GUILD_ANNOUNCEMENT,
	CHANNEL_ANNOUNCEMENT_THREAD = 10,
	CHANNEL_PUBLIC_THREAD,
	CHANNEL_PRIVATE_THREAD,
	CHANNEL_GUILD_STAGE_VOICE,
	CHANNEL_GUILD_DIRECTORY,
	CHANNEL_GUILD_FORUM
};

class cChannel final {
private:
	cSnowflake id;
	eChannelType type;
	// ...

public:
	explicit cChannel(const json::object&);
	explicit cChannel(const json::value&);

	const cSnowflake& GetId() const noexcept { return id; }
	eChannelType GetType() const noexcept { return type; }
};
typedef   hHandle<cChannel>   hChannel;
typedef  chHandle<cChannel>  chChannel;
typedef  uhHandle<cChannel>  uhChannel;
typedef uchHandle<cChannel> uchChannel;
typedef  shHandle<cChannel>  shChannel;
typedef schHandle<cChannel> schChannel;
#endif /* _GREEKBOT_CHANNEL_H_ */
