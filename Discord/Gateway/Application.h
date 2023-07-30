#ifndef GREEKBOT_APPLICATION_H
#define GREEKBOT_APPLICATION_H
#include "Common.h"

enum eApplicationFlag {
	APP_FLAG_GATEWAY_PRESENCE                 = 1 << 12, // Intent required for bots in 100 or more servers to receive presence_update events
	APP_FLAG_GATEWAY_PRESENCE_LIMITED         = 1 << 13, // Intent required for bots in under 100 servers to receive presence_update events
	APP_FLAG_GATEWAY_GUILD_MEMBERS            = 1 << 14, // Intent required for bots in 100 or more servers to receive member-related events
	APP_FLAG_GATEWAY_GUILD_MEMBERS_LIMITED    = 1 << 15, // Intent required for bots in under 100 servers to receive member-related events
	APP_FLAG_VERIFICATION_PENDING_GUILD_LIMIT = 1 << 16, // Indicates unusual growth of an app that prevents verification
	APP_FLAG_EMBEDDED                         = 1 << 17, // Indicates if an app is embedded within the Discord client (currently unavailable publicly)
	APP_FLAG_GATEWAY_MESSAGE_CONTENT          = 1 << 18, // Intent required for bots in 100 or more servers to receive message content
	APP_FLAG_GATEWAY_MESSAGE_CONTENT_LIMITED  = 1 << 19, // Intent required for bots in under 100 servers to receive message content
	APP_FLAG_APPLICATION_COMMAND_BADGE        = 1 << 23  // Indicates if an app has registered global application commands
};
inline eApplicationFlag operator|(eApplicationFlag a, eApplicationFlag b) { return (eApplicationFlag)((int)a | (int)b); }
inline eApplicationFlag operator&(eApplicationFlag a, eApplicationFlag b) { return (eApplicationFlag)((int)a & (int)b); }

eApplicationFlag tag_invoke(boost::json::value_to_tag<eApplicationFlag>, const boost::json::value&);

/* Partially implemented application object */
class cApplication {
private:
	cSnowflake m_id;
	eApplicationFlag m_flags;

	cApplication(const boost::json::value&);

public:
	const cSnowflake& GetId()    const noexcept { return m_id;    }
	eApplicationFlag  GetFlags() const noexcept { return m_flags; }

	friend cApplication tag_invoke(boost::json::value_to_tag<cApplication>, const boost::json::value&);
};
cApplication tag_invoke(boost::json::value_to_tag<cApplication>, const boost::json::value&);
#endif //GREEKBOT_APPLICATION_H