#ifndef GREEKBOT_CHANNEL_H
#define GREEKBOT_CHANNEL_H
#include "Base.h"

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
};
typedef   hHandle<cChannel>   hChannel;
typedef  chHandle<cChannel>  chChannel;
typedef  uhHandle<cChannel>  uhChannel;
typedef uchHandle<cChannel> uchChannel;

class crefChannel final {
	const cSnowflake& m_id;
public:
	crefChannel(const cSnowflake& id) noexcept : m_id(id) {}
	crefChannel(const cChannel& channel) noexcept : m_id(channel.GetId()) {}

	const cSnowflake& GetId() const noexcept { return m_id; }
};
#endif /* GREEKBOT_CHANNEL_H */
