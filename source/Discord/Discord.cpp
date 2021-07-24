#include "Discord.h"
#include "Net.h"

#if 0
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
#endif

hGatewayInfo cDiscord::GetGatewayInfo(const char *http_auth) {
	try {
		/* Make HTTP request to the gateway endpoint */
		std::string http_response;
		if (!cNet::GetHttpsRequest(DISCORD_API_HOST, DISCORD_API_GATEWAY_BOT, http_auth, http_response))
			return nullptr;
		
		/* Parse request response as JSON */
		json::monotonic_resource mr;
		json::stream_parser p(&mr);
		p.write(http_response);
		
		/* Construct gateway info object from JSON */
		return new cGatewayInfo(p.release());
	}
	catch (const std::exception&) {
		return nullptr;
	}
}
