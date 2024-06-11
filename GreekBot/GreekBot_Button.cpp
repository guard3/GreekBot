#include "GreekBot.h"

static const cMessageParams MISSING_PERMISSION_MESSAGE = [] {
	cMessageParams result;
	result.SetFlags(MESSAGE_FLAG_EPHEMERAL);
	result.SetContent("You can't do that. You're missing the `MANAGE_NICKNAMES` permission.");
	return result;
}();

cTask<>
cGreekBot::process_nickname_button(cMsgCompInteraction& i, const cSnowflake& user_id) HANDLER_BEGIN {
	/* Make sure that the invoking member has the appropriate permissions */
	if (!(i.GetMember()->GetPermissions() & PERM_MANAGE_NICKNAMES))
		co_return co_await InteractionSendMessage(i, MISSING_PERMISSION_MESSAGE);
	/* Respond with a modal */
	co_await InteractionSendModal(i, cModal{ "NICKNAME_MODAL", "Assign a nickname!", {
		cActionRow{
			cTextInput{
				TEXT_INPUT_SHORT,
				user_id.ToString(), // Save the member id
				"Greek nickname"
			}.SetMaxLength(30) // Nicknames can be up to 30 characters
		}}
	});
} HANDLER_END

cTask<>
cGreekBot::process_modal(cModalSubmitInteraction& i) HANDLER_BEGIN {
	/* Make sure, yet again, that the invoking member has the appropriate permissions */
	if (!(i.GetMember()->GetPermissions() & PERM_MANAGE_NICKNAMES))
		co_return co_await InteractionSendMessage(i, MISSING_PERMISSION_MESSAGE);
	/* Acknowledge the interaction immediately */
	co_await InteractionDefer(i);
	/* Modify the member referenced in the modal */
	auto& text_input = std::get<cTextInput>(i.GetComponents().front().GetComponents().front());
	co_await ModifyGuildMember(*i.GetGuildId(), text_input.GetCustomId(), cMemberOptions().SetNick(text_input.MoveValue()));
} HANDLER_END