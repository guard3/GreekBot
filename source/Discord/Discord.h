#pragma once
#ifndef _GREEKBOT_DISCORD_H_
#define _GREEKBOT_DISCORD_H_
#include "json.h"
#include "GatewayInfo.h"

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
