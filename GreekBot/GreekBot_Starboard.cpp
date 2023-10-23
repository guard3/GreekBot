#include "GreekBot.h"
#include "Utils.h"

cTask<>
cGreekBot::OnMessageReactionAdd(cSnowflake& user_id, cSnowflake& channel_id, cSnowflake& message_id, hSnowflake guild_id, hSnowflake message_author_id, hMember member, cEmoji& emoji) {
	co_return;
}

cTask<>
cGreekBot::OnMessageReactionRemove(cSnowflake& user_id, cSnowflake& channel_id, cSnowflake& message_id, hSnowflake guild_id, cEmoji& emoji) {
	co_return;
}

cTask<>
cGreekBot::OnMessageReactionRemoveAll(cSnowflake& channel_id, cSnowflake& message_id, hSnowflake guild_id) {
	co_return;
}

cTask<>
cGreekBot::OnMessageReactionRemoveEmoji(cSnowflake& channel_id, cSnowflake& message_id, hSnowflake guild_id, cEmoji& emoji) {
	co_return;
}