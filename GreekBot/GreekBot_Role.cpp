#include "GreekBot.h"
#include "Utils.h"
#include <algorithm>
#include <iterator>
/* ================================================================================================================== */
namespace rng = std::ranges;
/* ================================================================================================================== */
enum : uint32_t {
	CMP_ID_HERITAGE_SPEAKER    = 0x21A595C7,
	CMP_ID_IPA_LITERATE        = 0x12A20352,
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
	ROLE_ID_FLUENT             = 350483489461895168,  // @Fluent
	ROLE_ID_NATIVE             = 350483752490631181,  // @Native
	ROLE_ID_ADVANCED           = 350485279238258689,  // @Advanced
	ROLE_ID_INTERMEDIATE       = 350485376109903882,  // @Intermediate
	ROLE_ID_BEGINNER           = 351117824300679169,  // @Beginner
	ROLE_ID_ELEMENTARY         = 351117954974482435,  // @Elementary
	ROLE_ID_UPPER_INTERMEDIATE = 351118486426091521,  // @Upper Intermediate
	ROLE_ID_NON_LEARNER        = 352001527780474881,  // @Non Learner
	ROLE_ID_CYPRUS             = 1247132108686884894, // @Cyprus
};
/* ================================================================================================================== */
cTask<>
cGreekBot::process_role_button(cMsgCompInteraction& i, cSnowflake selected_role_id) HANDLER_BEGIN {
	static constexpr std::uint64_t valid_roles[] {
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
	static_assert(rng::is_sorted(valid_roles), "Must be sorted for binary search");
	/* Prepare response */
	cPartialMessage response;
	response.SetFlags(MESSAGE_FLAG_EPHEMERAL);
	const auto member_role_ids = i.GetMember()->GetRoles();
	/* Validate selected role */
	if (selected_role_id.ToInt() == 650330610358943755) {
		/* @Poll is only available for natives */
		if (rng::find(member_role_ids, ROLE_ID_NATIVE, &cSnowflake::ToInt) == member_role_ids.end())
			co_return co_await InteractionSendMessage(i, response.SetContent(fmt::format("Sorry, <@&{}> is only available for <@{}>s!", selected_role_id, (std::uint64_t)ROLE_ID_NATIVE)));
	} else if (!rng::binary_search(valid_roles, selected_role_id.ToInt())) {
		throw std::runtime_error(fmt::format("Role id {} was unexpected", selected_role_id));
	}
	/* Give or take away the selected role */
	co_await InteractionDefer(i);
	if (rng::find(member_role_ids, selected_role_id) == member_role_ids.end()) {
		co_await AddGuildMemberRole(*i.GetGuildId(), i.GetUser(), selected_role_id);
		response.SetContent(fmt::format("I gave you the <@&{}> role!", selected_role_id));
	} else {
		co_await RemoveGuildMemberRole(LMG_GUILD_ID, i.GetUser(), selected_role_id);
		response.SetContent(fmt::format("I took away your <@&{}> role!", selected_role_id));
	}
	co_await InteractionSendMessage(i, response);
} HANDLER_END
/* ================================================================================================================== */
cTask<>
cGreekBot::process_proficiency_menu(cMsgCompInteraction& i) HANDLER_BEGIN {
	/* The proficiency roles of Learning Greek; sorted for binary search to work! */
	static constexpr std::uint64_t pr_roles[] {
		ROLE_ID_FLUENT,             // 350483489461895168  @Fluent
		ROLE_ID_NATIVE,             // 350483752490631181  @Native
		ROLE_ID_ADVANCED,           // 350485279238258689  @Advanced
		ROLE_ID_INTERMEDIATE,       // 350485376109903882  @Intermediate
		ROLE_ID_BEGINNER,           // 351117824300679169  @Beginner
		ROLE_ID_ELEMENTARY,         // 351117954974482435  @Elementary
		ROLE_ID_UPPER_INTERMEDIATE, // 351118486426091521  @Upper Intermediate
		ROLE_ID_NON_LEARNER,        // 352001527780474881  @Non Learner
		ROLE_ID_CYPRUS              // 1247132108686884894 @Cyprus
	};
	static_assert(rng::is_sorted(pr_roles), "Must be sorted for binary search");
	co_await InteractionDefer(i);
	auto member_roles = i.GetMember()->GetRoles();
	/* Add roles to member update options */
	cMemberOptions options;
	auto& roles = options.EmplaceRoles();
	roles.reserve(member_roles.size() + 2);
	/* Copy all member roles, except the proficiency ones */
	rng::copy_if(member_roles, std::back_inserter(roles), [](cSnowflake& role_id) {
		return !rng::binary_search(pr_roles, role_id.ToInt());
	});
	/* Append the selected role id */
	cSnowflake selected_role_id = i.GetValues().front();
	roles.push_back(selected_role_id);
	/* If the user chose "Cyprus", also give them "Native" */
	if (selected_role_id == ROLE_ID_CYPRUS)
		roles.emplace_back(ROLE_ID_NATIVE);
	/* Update member and send confirmation message */
	co_await ModifyGuildMember(*i.GetGuildId(), i.GetUser().GetId(), options);
	co_await InteractionSendMessage(i, cPartialMessage()
		.SetFlags(MESSAGE_FLAG_EPHEMERAL)
		.SetContent(fmt::format("I gave you the <@&{}> role!", selected_role_id))
	);
} HANDLER_END
/* ================================================================================================================== */
cTask<>
cGreekBot::process_booster_menu(cMsgCompInteraction& i) HANDLER_BEGIN {
	using namespace std::chrono;
	/* All the color roles available in Learning Greek; sorted for binary search to work */
	static constexpr std::uint64_t color_roles[] {
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
	static_assert(rng::is_sorted(color_roles), "Must be sorted for binary search");
	/* Acknowledge interaction */
	co_await InteractionDefer(i);
	/* Prepare confirmation message */
	cPartialMessage response;
	response.SetFlags(MESSAGE_FLAG_EPHEMERAL);
	/* Create a new role vector */
	auto member = i.GetMember();
	auto member_roles = member->GetRoles();
	cMemberOptions options;
	auto& roles = options.EmplaceRoles();
	roles.reserve(member_roles.size() + 1);
	/* Copy all roles, except the color ones */
	rng::copy_if(member_roles, std::back_inserter(roles), [](cSnowflake& role_id) {
		return !rng::binary_search(color_roles, role_id.ToInt());
	});
	/* Retrieve the selected role id */
	cSnowflake selected_id = i.GetValues().front();
	/* Include the selected role id if the user is boosting */
	if (member->PremiumSince() > sys_seconds{}) {
		if (selected_id != 0)
			roles.push_back(selected_id);
		/* Update member and send confirmation message */
		co_await ModifyGuildMember(LMG_GUILD_ID, i.GetUser().GetId(), options);
		if (selected_id != 0)
			response.SetContent(fmt::format("I gave you the <@&{}> role!", selected_id));
		else if (roles.size() < member_roles.size())
			response.SetContent("I took away your color role!");
		else co_return;
		co_return co_await InteractionSendMessage(i, response);
	}
	if (selected_id != 0)
		co_await InteractionSendMessage(i, response.SetContent("Sorry, custom colors are only available for <@&593038680608735233>s!"));
} HANDLER_END