#include "GreekBot.h"
#include "Utils.h"
#include <algorithm>
/* ================================================================================================================== */
enum : uint32_t {
	CMP_ID_HERITAGE_SPEAKER    = 0x6691F68D,
	CMP_ID_IPA_LITERATE        = 0x59A60FDC,
	CMP_ID_STUDENT             = 0x88665129,
	CMP_ID_WORD_OF_THE_DAY     = 0x91067435,
	CMP_ID_EVENTS              = 0xA37F90CC,
	CMP_ID_DIALECT             = 0xB2C24CDD,
	CMP_ID_GAMER               = 0x7F70D917,
	CMP_ID_LINGUISTICS_READING = 0x5F0C7CB1,
	CMP_ID_POLL                = 0xB22917F1,
	CMP_ID_VCER                = 0xB5953763,
	CMP_ID_BOOK_CLUB           = 0x24704291
};
/* ================================================================================================================== */
enum : uint64_t {
	ROLE_ID_NATIVE             = 350483752490631181, // @Native
	ROLE_ID_BEGINNER           = 351117824300679169, // @Beginner
	ROLE_ID_ELEMENTARY         = 351117954974482435, // @Elementary
	ROLE_ID_INTERMEDIATE       = 350485376109903882, // @Intermediate
	ROLE_ID_UPPER_INTERMEDIATE = 351118486426091521, // @Upper Intermediate
	ROLE_ID_ADVANCED           = 350485279238258689, // @Advanced
	ROLE_ID_FLUENT             = 350483489461895168, // @Fluent
	ROLE_ID_NON_LEARNER        = 352001527780474881  // @Non Learner
};
/* ================================================================================================================== */
cTask<>
cGreekBot::process_role_button(cMsgCompInteraction& i, uint32_t button_id) {
	co_await RespondToInteraction(i);

	chMember member = i.GetMember();
	auto roles = member->GetRoles();

	uint64_t role_id;
	switch (button_id) {
		case CMP_ID_HERITAGE_SPEAKER:
			role_id = 762747975164100608;
			break;
		case CMP_ID_IPA_LITERATE:
			role_id = 481544681520365577;
			break;
		case CMP_ID_STUDENT:
			role_id = 469239964119597056;
			break;
		case CMP_ID_WORD_OF_THE_DAY:
			role_id = 649313942002466826;
			break;
		case CMP_ID_EVENTS:
			role_id = 683443550410506273;
			break;
		case CMP_ID_DIALECT:
			role_id = 637359870009409576;
			break;
		case CMP_ID_GAMER:
			role_id = 357714047698993162;
			break;
		case CMP_ID_LINGUISTICS_READING:
			role_id = 800464173385515058;
			break;
		case CMP_ID_POLL:
			role_id = 650330610358943755;
			if (std::find(roles.begin(), roles.end(), ROLE_ID_NATIVE) == roles.end()) {
				co_return co_await SendInteractionFollowupMessage(i,
					kw::flags=MESSAGE_FLAG_EPHEMERAL,
					kw::content=fmt::format("Sorry, <@&{}> is only available for <@{}>s!", role_id, (uint64_t)ROLE_ID_NATIVE)
				);
			}
			break;
		case CMP_ID_VCER:
			role_id = 886625423167979541;
			break;
		case CMP_ID_BOOK_CLUB:
			role_id = 928364890601685033;
			break;
		default:
			cUtils::PrintLog("COMPONENT ID: 0x{:08X}", button_id);
			co_return;
	}

	if (std::find(roles.begin(), roles.end(), role_id) == roles.end())
		co_await AddGuildMemberRole(m_lmg_id, member->GetUser()->GetId(), role_id);
	else
		co_await RemoveGuildMemberRole(m_lmg_id, member->GetUser()->GetId(), role_id);
}
/* ================================================================================================================== */
cTask<>
cGreekBot::process_proficiency_menu(cMsgCompInteraction& i) {
	/* The proficiency roles of Learning Greek */
	static const cSnowflake pr_roles[] {
		ROLE_ID_NATIVE,
		ROLE_ID_BEGINNER,
		ROLE_ID_ELEMENTARY,
		ROLE_ID_INTERMEDIATE,
		ROLE_ID_UPPER_INTERMEDIATE,
		ROLE_ID_ADVANCED,
		ROLE_ID_FLUENT,
		ROLE_ID_NON_LEARNER
	};
	/* Acknowledge interaction */
	co_await RespondToInteraction(i);
	/* Create a role vector */
	chMember member = i.GetMember();
	std::vector<chSnowflake> roles;
	roles.reserve(member->GetRoles().size() + 1);
	/* Copy every member role except the proficiency ones */
	for (auto& s : member->GetRoles()) {
		if (std::find(std::begin(pr_roles), std::end(pr_roles), s) == std::end(pr_roles))
			roles.push_back(&s);
	}
	/* Append the selected role id */
	cSnowflake id = i.GetValues().front();
	roles.push_back(&id);
	/* Update member */
	co_await UpdateGuildMemberRoles(m_lmg_id, member->GetUser()->GetId(), roles);
}
/* ================================================================================================================== */
cTask<>
cGreekBot::process_booster_menu(cMsgCompInteraction& i) {
	using namespace std::chrono_literals;
	/* All the color roles available in Learning Greek */
	static const cSnowflake color_roles[] {
		777323857018617877,  // @Μπεκρής
		657145859439329280,  // @Κακούλης
		677183404743327764,  // @Πορτοκαλί
		735954079355895889,  // @Μέλας
		585252617278586880,  // @Ροζουλής
		1109212629882392586, // @Τούρκος
		941041008169336913,  // @Πολωνός
		755763454266179595,  // @Λεμόνι
		1156980445058170991, // @Πίκατσου
		793570278084968488,  // @Γύπας
		925379778251485206,  // @Δογκ
		1121773567785308181  // @Πέγκω
	};
	/* Acknowledge interaction */
	co_await RespondToInteraction(i);
	/* Create a new role vector */
	chMember member = i.GetMember();
	std::vector<chSnowflake> roles;
	roles.reserve(member->GetRoles().size() + 1);
	/* Fill the vector with all the roles the member has except the color ones */
	for (auto& id : member->GetRoles()) {
		if (std::find(std::begin(color_roles), std::end(color_roles), id) == std::end(color_roles))
			roles.push_back(&id);
	}
	/* Retrieve the selected role id */
	cSnowflake selected_id = i.GetValues().front();
	/* Include the selected role id if the user is boosting */
	if (member->PremiumSince().time_since_epoch() > 0s) {
		if (selected_id != 0) {
			roles.push_back(&selected_id);
		}
		co_return co_await UpdateGuildMemberRoles(m_lmg_id, member->GetUser()->GetId(), roles);
	}
	if (selected_id != 0)
		co_await SendInteractionFollowupMessage(i, kw::flags=MESSAGE_FLAG_EPHEMERAL, kw::content="Sorry, custom colors are only available for <@&593038680608735233>s!");
}