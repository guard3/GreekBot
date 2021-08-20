#include "Discord.h"
#include "Net.h"

uchGatewayInfo cDiscord::GetGatewayInfo(const char *http_auth, uchError& error) {
	/* Reset error handle at first */
	error.reset();

	/* Make HTTP request to the gateway endpoint */
	std::string http_response;
	if (!cNet::GetHttpsRequest(DISCORD_API_HOST, DISCORD_API_GATEWAY_BOT, http_auth, http_response))
		return uchGatewayInfo();

	json::value value;
	try {
		/* Parse request response as JSON */
		json::monotonic_resource mr;
		json::stream_parser p(&mr);
		p.write(http_response);
		value = p.release();
		
		/* Construct gateway info object */
		return std::make_unique<cGatewayInfo>(value);
	}
	catch (const std::exception&) {
		try {
			error = std::make_unique<cError>(value);
		}
		catch (const std::exception&) {}
		return uchGatewayInfo();
	}
}