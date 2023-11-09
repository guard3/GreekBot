#include "GreekBot.h"

cTask<>
cGreekBot::process_nickname_button(cMsgCompInteraction& i, const cSnowflake& user_id) {
	/* Make sure that the invoking member has the appropriate permissions */
	if (!(i.GetMember()->GetPermissions() & PERM_MANAGE_NICKNAMES)) {
		co_await RespondToInteraction(i);
		co_return co_await SendInteractionFollowupMessage(i,
			kw::flags=MESSAGE_FLAG_EPHEMERAL,
			kw::content="You can't do that. You're missing the `MANAGE_NICKNAMES` permission."
		);
	}
	/* Respond with a modal */
	co_await RespondToInteractionWithModal(i, cModal{
		"null", // We don't depend on a custom id since this is the only modal sent, at least at the moment, so we use that, YOLO
		"Assign a nickname!", {
			cActionRow{
				cTextInput{
					TEXT_INPUT_SHORT,
					user_id.ToString(), // Save the member id
					"Greek nickname",
					kw::min_length=1, //
					kw::max_length=30 // Nicknames can be up to 30 characters
				}
			}
		}
	});
}

cTask<>
cGreekBot::process_modal(cModalSubmitInteraction& i) {
	/* Acknowledge the interaction immediately */
	co_await RespondToInteraction(i);
	/* Make sure, yet again, that the invoking member has the appropriate permissions */
	if (!(i.GetMember()->GetPermissions() & PERM_MANAGE_NICKNAMES)) {
		co_return co_await SendInteractionFollowupMessage(i,
			kw::flags=MESSAGE_FLAG_EPHEMERAL,
			kw::content="You can't do that. You're missing the `MANAGE_NICKNAMES` permission."
		);
	}
	/* Modify the member referenced in the modal */
	try {
		auto& text_input = std::get<cTextInput>(i.GetComponents().front().GetComponents().front());
		co_return co_await ModifyGuildMember(m_lmg_id, text_input.GetCustomId(), kw::nick = text_input.GetValue());
	} catch (...) {}
	/* If member couldn't be modified for whatever reason, send an error message */
	co_await SendInteractionFollowupMessage(i,
		kw::flags=MESSAGE_FLAG_EPHEMERAL,
		kw::content="Something went wrong. Check your input and try again."
	);
}