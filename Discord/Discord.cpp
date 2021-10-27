#include "Discord.h"
#include "Net.h"
#include <thread>

unsigned int cDiscord::GetHttpsRequest(const char *path, const char *auth, std::string &response, cDiscordError &error) {
	for (;;) {
		switch (unsigned int status = cNet::GetHttpsRequest(DISCORD_API_HOST, path, auth, response)) {
			case 200:
				error = cDiscordError::MakeError<DISCORD_ERROR_NO_ERROR>();
				return status;
			default:
				try {
					json::monotonic_resource mr;
					json::stream_parser p(&mr);
					p.write(response);
					auto v = p.release();
					if (status == 429) {
						std::this_thread::sleep_for(std::chrono::milliseconds((int)(v.at("retry_after").as_double() * 1000.0)));
						break;
					}
					error = cDiscordError::MakeError<DISCORD_ERROR_HTTP>(v);
					return status;
				}
				catch (...) {}
				response.clear();
			case 0:
				error = cDiscordError::MakeError<DISCORD_ERROR_GENERIC>();
				return status;
		}
	}
}