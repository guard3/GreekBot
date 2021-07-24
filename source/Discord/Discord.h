#pragma once
#ifndef _GREEKBOT_DISCORD_H_
#define _GREEKBOT_DISCORD_H_
#include "json.h"
#include "GatewayInfo.h"

#define _STR(x) #x
#define STR(x) _STR(x)

#define DISCORD_API_VERSION     9
#define DISCORD_API_HOST        "discord.com"
#define DISCORD_API_ENDPOINT    "/api/v" STR(DISCORD_API_VERSION)
#define DISCORD_API_GATEWAY     DISCORD_API_ENDPOINT "/gateway"
#define DISCORD_API_GATEWAY_BOT DISCORD_API_GATEWAY  "/bot"

/* A helper class that performs HTTP requests to the API endpoint */
class cDiscord final {
private:
	cDiscord() {}
	
public:
	static bool GetGateway(const char* auth, json::value& value);
	
	static hGatewayInfo GetGatewayInfo(const char* http_auth);
	
	template<typename H>
	static void CloseHandle(H h) { delete h; }
};

#endif /* _GREEKBOT_DISCORD_H_*/
