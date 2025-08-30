#ifndef DISCORD_CHANNEL_H
#define DISCORD_CHANNEL_H
#include "Base.h"
#include "ChannelFwd.h"

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
eChannelType tag_invoke(boost::json::value_to_tag<eChannelType>, const boost::json::value&);

class cChannel final {
private:
	cSnowflake id;
	eChannelType type;
	// ...

public:
	explicit cChannel(const boost::json::object&);
	explicit cChannel(const boost::json::value&);

	const cSnowflake& GetId() const noexcept { return id; }
	eChannelType GetType() const noexcept { return type; }

	/**
	 * Allow implicit conversion to crefChannel
	 */
	operator crefChannel() const noexcept { return id; }
};
#endif /* DISCORD_CHANNEL_H */