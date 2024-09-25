#include "GreekBot.h"

static const auto MISSING_PERMISSION_MSG = [] {
	cPartialMessage msg;
	msg.SetFlags(MESSAGE_FLAG_EPHEMERAL).SetContent("You can't do that. You're missing the `MANAGE_MESSAGES` permission");
	return msg;
}();

cTask<>
cGreekBot::process_clear(cAppCmdInteraction& i) {
	/* Make sure that the invoking member has the appropriate permissions */
	if (!(i.GetMember()->GetPermissions() & PERM_MANAGE_MESSAGES))
		co_return co_await InteractionSendMessage(i, MISSING_PERMISSION_MSG);
	/* Retrieve how many messages to delete, making sure it's in [1, 100] range */
	auto num = std::clamp(i.GetOptions().front().GetValue<APP_CMD_OPT_INTEGER>(), 1, 100);
	/* Start retrieving channel messages */
	std::vector<cSnowflake> ids;
	ids.reserve(100);
	auto gen = GetChannelMessages(i.GetChannelId(), num);
	/* Run the generator once to start the process first... */
	auto it = co_await gen.begin();
	/* ...and defer the interaction later, to avoid getting this response in the list of the messages to be deleted! */
	co_await InteractionDefer(i);
	/* Consume the generator and save all message ids */
	for (; it != gen.end(); co_await ++it)
		ids.push_back(it->GetId());
	/* Delete messages */
	cPartialMessage response;
	if (ids.empty()) {
		response.SetContent("There are no messages to delete.");
	} else {
		co_await DeleteMessages(i.GetChannelId(), ids, "/clear command");
		response.SetContent(std::format("I deleted **{}** message{}<a:mop:1255169498051379361>", ids.size(), ids.size() == 1 ? "" : "s"));
		response.SetComponents({
			cActionRow{
				cButton{
					BUTTON_STYLE_SECONDARY,
					std::format("DLT#{}", i.GetUser().GetId()),
					"Dismiss"
				}
			}
		});
	}
	co_await InteractionSendMessage(i, response);
}

cTask<>
cGreekBot::process_dismiss(cMsgCompInteraction& i, cSnowflake user_id) HANDLER_BEGIN {
	/* If we're on a guild, check if the user is the original author or if they have appropriate permissions */
	if (i.GetUser().GetId() != user_id && i.GetMember() && !(i.GetMember()->GetPermissions() & PERM_MANAGE_MESSAGES))
		co_return co_await InteractionSendMessage(i, MISSING_PERMISSION_MSG);
	/* If all good, delete original message */
	co_await InteractionDefer(i);
	co_await InteractionDeleteMessage(i, i.GetMessage());
} HANDLER_END