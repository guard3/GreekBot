#include "GreekBot.h"

cTask<>
cGreekBot::process_nickname_button(cMsgCompInteraction& i, const cSnowflake& user_id) {
	try {
		/* Make sure that the invoking member has the appropriate permissions */
		if (!(i.GetMember()->GetPermissions() & PERM_MANAGE_NICKNAMES)) {
			co_return co_await InteractionSendMessage(i, cMessageParams{
				kw::flags=MESSAGE_FLAG_EPHEMERAL,
				kw::content="You can't do that. You're missing the `MANAGE_NICKNAMES` permission."
			});
		}
		/* Respond with a modal */
		co_return co_await InteractionSendModal(i, cModal{
			"NICKNAME_MODAL",
			"Assign a nickname!", {
				cActionRow{
					cTextInput{
						TEXT_INPUT_SHORT,
						user_id.ToString(), // Save the member id
						"Greek nickname",
						kw::min_length = 1, //
						kw::max_length = 30 // Nicknames can be up to 30 characters
					}
				}
			}
		});
	} catch (...) {
		report_error("process_nickname_button", std::current_exception());
	}
	co_await InteractionSendMessage(i, cMessageParams{
		kw::flags=MESSAGE_FLAG_EPHEMERAL,
		kw::content="An unexpected error has occurred. Try again later."
	});
}

cTask<>
cGreekBot::process_modal(cModalSubmitInteraction& i) {
	try {
		/* Make sure, yet again, that the invoking member has the appropriate permissions */
		if (!(i.GetMember()->GetPermissions() & PERM_MANAGE_NICKNAMES)) {
			co_return co_await InteractionSendMessage(i, cMessageParams{
				kw::flags=MESSAGE_FLAG_EPHEMERAL,
				kw::content="You can't do that. You're missing the `MANAGE_NICKNAMES` permission."
			});
		}
		/* Acknowledge the interaction immediately */
		co_await InteractionDefer(i);
		/* Modify the member referenced in the modal */
		auto &text_input = std::get<cTextInput>(i.GetComponents().front().GetComponents().front());
		co_await ModifyGuildMember(m_lmg_id, text_input.GetCustomId(), kw::nick = text_input.GetValue());
		co_return;
	} catch (...) {
		report_error("process_modal", std::current_exception());
	}
	/* If member couldn't be modified for whatever reason, send an error message */
	co_await InteractionSendMessage(i, cMessageParams{
		kw::flags = MESSAGE_FLAG_EPHEMERAL,
		kw::content = "Something went wrong. Check your input and try again."
	});
}