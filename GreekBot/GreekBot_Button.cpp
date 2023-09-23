#include "GreekBot.h"

cTask<>
cGreekBot::OnInteraction_button(const cInteraction& i) {
	co_await RespondToInteraction(i);
	co_await SendInteractionFollowupMessage(i, kw::flags=MESSAGE_FLAG_EPHEMERAL, kw::content="Every minute that you're messaging, you randomly gain between 15 and 25 **XP**.");
}

cTask<>
cGreekBot::process_nickname_button(const cInteraction& i, const cSnowflake& user_id) {
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
cGreekBot::process_modal(const cInteraction& i) {
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
	auto& text_input = i.GetData<INTERACTION_MODAL_SUBMIT>().GetSubmittedData().front();
	co_await ModifyGuildMember(m_lmg_id, text_input.GetCustomId(), kw::nick=text_input.GetValue());
}