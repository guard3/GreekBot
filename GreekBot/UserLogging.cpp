#include "GreekBot.h"
#include "CDN.h"

cPartialMessage
static make_message(const cUser& user, cColor col, std::string_view text) {
	using namespace std::chrono;

	cPartialMessage msg;
	auto& embed = msg.EmplaceEmbeds().emplace_back();
	embed.EmplaceAuthor(user.GetUsername()).SetIconUrl(cCDN::GetUserAvatar(user));
	embed.SetColor(col);
	embed.SetTimestamp(floor<milliseconds>(system_clock::now()));
	embed.SetDescription(text);
	embed.SetFields({{ "User ID", std::format("`{}`", user.GetId()) }});

	return msg;
}

cTask<> cGreekBot::process_usrlog_new_member(const cMember& member) HANDLER_BEGIN {
	co_await CreateMessage(LMG_CHANNEL_USER_LOG, make_message(*member.GetUser(), LMG_COLOR_GREEN, "🎉 Joined the server"));
} HANDLER_END

cTask<> cGreekBot::process_usrlog_member_remove(const cUser& user) HANDLER_BEGIN {
	co_await CreateMessage(LMG_CHANNEL_USER_LOG, make_message(user, LMG_COLOR_RED, "🚪 Left the server"));
} HANDLER_END
