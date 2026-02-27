#include "GreekBot.h"
#include <boost/url.hpp>

namespace urls = boost::urls;

cTask<>
cGreekBot::process_sanitize(cAppCmdInteraction& i) {
	cPartialMessage response;

	if (auto url = urls::parse_uri(i.GetOptions().front().GetValue<APP_CMD_OPT_STRING>()); url.has_error()) {
		response.SetContent("Please provide a youtube url.");
	} else if (auto host = url->encoded_host_name(); !host.ends_with("youtube.com") && !host.ends_with("youtu.be")
		                                          || url->scheme_id() != urls::scheme::https && url->scheme_id() != urls::scheme::http) {
		response.SetContent("Please provide a youtube url.");
	} else if (urls::url clean_url = *url; clean_url.params().erase("si", urls::ignore_case) == 0) {
		response.SetContent("Your youtube link is already clean!");
	} else {
		response.SetContent(clean_url.buffer());
	}

	co_await InteractionSendMessage(i, response.SetFlags(MESSAGE_FLAG_EPHEMERAL));
}
