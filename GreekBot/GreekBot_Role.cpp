#include "GreekBot.h"
#include <algorithm>
#include <ranges>

cTask<>
cGreekBot::process_role_button(cMsgCompInteraction& i, cSnowflake selected_role_id) HANDLER_BEGIN {
	static constexpr cSnowflake valid_roles[] {
		357714047698993162, // @Gamer
		469239964119597056, // @Student
		481544681520365577, // @IPA Literate
		637359870009409576, // @Dialect
		649313942002466826, // @Word of the day
		683443550410506273, // @Events
		762747975164100608, // @Heritage speaker
		800464173385515058, // @Linguistics Reading
		886625423167979541, // @VCer
		928364890601685033, // @Book Club
	};
	/* Prepare response */
	cPartialMessage response;
	response.SetFlags(MESSAGE_FLAG_EPHEMERAL);
	const auto member_role_ids = i.GetMember()->GetRoles();
	/* Validate selected role */
	if (selected_role_id == LMG_ROLE_POLL) {
		/* @Poll is only available for natives */
		if (!std::ranges::contains(member_role_ids, LMG_ROLE_NATIVE))
			co_return co_await InteractionSendMessage(i, response.SetContent(std::format("Sorry, <@&{}> is only available for <@&{}>s!", selected_role_id, LMG_ROLE_NATIVE)));
	} else if (!std::ranges::contains(valid_roles, selected_role_id)) {
		throw std::runtime_error(std::format("Role id {} was unexpected", selected_role_id));
	}
	/* Give or take away the selected role */
	co_await InteractionDefer(i);
	if (std::ranges::contains(member_role_ids, selected_role_id)) {
		co_await RemoveGuildMemberRole(LMG_GUILD_ID, i.GetUser(), selected_role_id);
		response.SetContent(std::format("I took away your <@&{}> role!", selected_role_id));
	} else {
		co_await AddGuildMemberRole(*i.GetGuildId(), i.GetUser(), selected_role_id);
		response.SetContent(std::format("I gave you the <@&{}> role!", selected_role_id));
	}
	co_await InteractionSendMessage(i, response);
} HANDLER_END
/* ================================================================================================================== */
cTask<>
cGreekBot::process_proficiency_menu(cMsgCompInteraction& i) HANDLER_BEGIN {
	co_await InteractionDefer(i);

	/* Filter out any existing proficiency roles */
	std::vector roles = i.GetMember()->GetRoles()
	                  | std::views::filter([](auto& id) { return !std::ranges::contains(LMG_PROFICIENCY_ROLES, id); })
	                  | std::ranges::to<std::vector>();

	/* Append the selected role id */
	cSnowflake selected_role = i.GetValues().front();
	roles.push_back(selected_role);

	/* If the user chose "Cyprus", also give them "Native" */
	if (selected_role == LMG_ROLE_CYPRUS)
		roles.push_back(LMG_ROLE_NATIVE);

	/* Update member and send a confirmation message */
	co_await ModifyGuildMember(*i.GetGuildId(), i.GetUser().GetId(), cMemberOptions().SetRoles(std::move(roles)));
	co_await InteractionSendMessage(i, cPartialMessage()
		.SetFlags(MESSAGE_FLAG_EPHEMERAL)
		.SetContent(std::format("I gave you the <@&{}> role!", selected_role))
	);
} HANDLER_END
/* ================================================================================================================== */
cTask<>
cGreekBot::process_booster_menu(cMsgCompInteraction& i) HANDLER_BEGIN {
	using namespace std::chrono;
	/* All the color roles available in Learning Greek */
	static constexpr cSnowflake color_roles[] {
		 657145859439329280, // @Κακούλης
		 677183404743327764, // @Πορτοκαλί
		 735954079355895889, // @Μέλας
		 755763454266179595, // @Λεμόνι
		 777323857018617877, // @Μπεκρής
		 793570278084968488, // @Γύπας
		 925379778251485206, // @Ροζουλής
		 941041008169336913, // @Πολωνός
		1109212629882392586, // @Τούρκος
		1121773567785308181, // @Πέγκω
		1156980445058170991, // @Κυνεζί
		1163945469567832215, // @Πάπια
		1265742860783845558  // @Κατσίκα
	};

	/* Acknowledge interaction */
	co_await InteractionDefer(i);
	/* Prepare the confirmation message */
	cPartialMessage response;
	response.SetFlags(MESSAGE_FLAG_EPHEMERAL);
	/* Create a new role vector */
	auto member = i.GetMember();
	auto member_roles = member->GetRoles();

	std::vector roles = member_roles
	                  | std::views::filter([](auto& id) { return !std::ranges::contains(color_roles, id); })
	                  | std::ranges::to<std::vector>();

	/* Retrieve the selected role id */
	cSnowflake selected_id = i.GetValues().front();
	/* Include the selected role id if the user is boosting */
	if (member->PremiumSince() > sys_seconds{}) {
		if (selected_id != 0)
			roles.push_back(selected_id);
		/* Update member and send the confirmation message */
		co_await ModifyGuildMember(LMG_GUILD_ID, i.GetUser().GetId(), cMemberOptions().SetRoles(std::move(roles)));
		if (selected_id != 0)
			response.SetContent(std::format("I gave you the <@&{}> role!", selected_id));
		else if (roles.size() < member_roles.size())
			response.SetContent("I took away your color role!");
		else co_return;
		co_return co_await InteractionSendMessage(i, response);
	}
	if (selected_id != 0)
		co_await InteractionSendMessage(i, response.SetContent("Sorry, custom colors are only available for <@&593038680608735233>s!"));
} HANDLER_END