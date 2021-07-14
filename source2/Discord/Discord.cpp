#include "Discord.h"
#include "Utils.h"
#include <iostream>

#define _STR(x) #x
#define STR(x) _STR(x)

#define DISCORD_API_VERSION     9
#define DISCORD_API_HOST        "discord.com"
#define DISCORD_API_ENDPOINT    "/api/v" STR(DISCORD_API_VERSION)
#define DISCORD_API_GATEWAY     DISCORD_API_ENDPOINT "/gateway"
#define DISCORD_API_GATEWAY_BOT DISCORD_API_GATEWAY  "/bot"

bool cDiscord::GetGateway(const char* auth, json::value& value) {
	/* Make HTTP request to the gateway endpoint */
	std::string response = cUtils::GetHttpsRequest(DISCORD_API_HOST, DISCORD_API_GATEWAY_BOT, auth);
	if (response.empty()) {
		value = {};
		return false;
	}
	
	/* Parse request response as JSON */
	json::error_code e;
	value = json::parse(response, e);
	return !static_cast<bool>(e);
}
