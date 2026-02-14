#ifndef GREEKBOT_LEARNINGGREEK_H
#define GREEKBOT_LEARNINGGREEK_H
#include "Base.h"
#include <algorithm>
/* ========== The id of Learning Greek ============================================================================== */
inline constexpr cSnowflake LMG_GUILD_ID = 350234668680871946;
/* ========== Learning Greek channels =============================================================================== */
inline constexpr cSnowflake LMG_CHANNEL_USER_LOG    =  469274019565142017;
inline constexpr cSnowflake LMG_CHANNEL_MESSAGE_LOG =  539521989061378048;
inline constexpr cSnowflake LMG_CHANNEL_STARBOARD   =  978993330694266920;
inline constexpr cSnowflake LMG_CHANNEL_NEW_MEMBERS = 1143888492422770778;
/* ========== Learning Greek roles ================================================================================== */
inline constexpr cSnowflake LMG_ROLE_FLUENT               =  350483489461895168; // @Fluent
inline constexpr cSnowflake LMG_ROLE_NATIVE               =  350483752490631181; // @Native
inline constexpr cSnowflake LMG_ROLE_ADVANCED             =  350485279238258689; // @Advanced
inline constexpr cSnowflake LMG_ROLE_INTERMEDIATE         =  350485376109903882; // @Intermediate
inline constexpr cSnowflake LMG_ROLE_BEGINNER             =  351117824300679169; // @Beginner
inline constexpr cSnowflake LMG_ROLE_ELEMENTARY           =  351117954974482435; // @Elementary
inline constexpr cSnowflake LMG_ROLE_UPPER_INTERMEDIATE   =  351118486426091521; // @Upper Intermediate
inline constexpr cSnowflake LMG_ROLE_NON_LEARNER          =  352001527780474881; // @Non-Learner
inline constexpr cSnowflake LMG_ROLE_ACTIVE_USER          =  352009155864821760; // @Active User
inline constexpr cSnowflake LMG_ROLE_MEMBER               =  350264923806367754; // @Member
inline constexpr cSnowflake LMG_ROLE_ACTIVE_MEMBER        =  352008860904456192; // @Active Member
inline constexpr cSnowflake LMG_ROLE_REALLY_ACTIVE_MEMBER =  353354156783960075; // @Really Active Member
inline constexpr cSnowflake LMG_ROLE_USUAL_SUSPECT        =  410753563543732225; // @Usual Suspect
inline constexpr cSnowflake LMG_ROLE_GREEK_LOVER          =  410754841556549633; // @Greek Lover
inline constexpr cSnowflake LMG_ROLE_PROESTOS             =  414523335595130905; // @Προεστός
inline constexpr cSnowflake LMG_ROLE_EPITIMOS             =  466238555304230913; // @Επίτιμος
inline constexpr cSnowflake LMG_ROLE_ELLINOPOULO          =  608687509291008000; // @Ελληνόπουλο
inline constexpr cSnowflake LMG_ROLE_PALIOS               =  631559446782410752; // @Παλιός
inline constexpr cSnowflake LMG_ROLE_POLL                 =  650330610358943755; // @Poll
inline constexpr cSnowflake LMG_ROLE_VETERANOS            =  817337981979066398; // @Βετεράνος
inline constexpr cSnowflake LMG_ROLE_CYPRUS               = 1247132108686884894; // @Cyprus
/* ========== Learning Greek role groups ============================================================================ */
inline constexpr cSnowflake LMG_RANK_ROLES[] {
	LMG_ROLE_MEMBER,
	LMG_ROLE_ACTIVE_MEMBER,
	LMG_ROLE_ACTIVE_USER,
	LMG_ROLE_REALLY_ACTIVE_MEMBER,
	LMG_ROLE_USUAL_SUSPECT,
	LMG_ROLE_GREEK_LOVER,
	LMG_ROLE_PROESTOS,
	LMG_ROLE_EPITIMOS,
	LMG_ROLE_ELLINOPOULO,
	LMG_ROLE_PALIOS
};
static_assert(std::ranges::is_sorted(LMG_RANK_ROLES), "Must be sorted for binary search");

inline constexpr cSnowflake LMG_PROFICIENCY_ROLES[] {
	LMG_ROLE_NON_LEARNER,
	LMG_ROLE_BEGINNER,
	LMG_ROLE_ELEMENTARY,
	LMG_ROLE_INTERMEDIATE,
	LMG_ROLE_UPPER_INTERMEDIATE,
	LMG_ROLE_ADVANCED,
	LMG_ROLE_FLUENT,
	LMG_ROLE_NATIVE,
	LMG_ROLE_CYPRUS
};

inline constexpr cSnowflake LMG_NITRO_BOOSTER_COLOR_ROLES[] {
	 657145859439329280, // @Κακούλης
	 677183404743327764, // @Πορτοκαλί
	 735954079355895889, // @Μέλας
	 755763454266179595, // @Λεμόνι
	 777323857018617877, // @Μπεκρής
	 793570278084968488, // @Γύπας
	 925379778251485206, // @Ροζουλής
	 941041008169336913, // @Πολωνός
	1109212629882392586, // @Σκατούλης
	1121773567785308181, // @Πέγκω
	1156980445058170991, // @Κυνεζί
	1163945469567832215, // @Χαρδ
	1265742860783845558, // @Κατσίκα
};
/* ========== Emojis ================================================================================================ */
inline constexpr cSnowflake LMG_EMOJI_HOLY = 409075809723219969;
/* ========== Common colors ========================================================================================= */
inline constexpr cColor LMG_COLOR_RED         = 0xC43135;
inline constexpr cColor LMG_COLOR_GREEN       = 0x248046;
inline constexpr cColor LMG_COLOR_BLUE        = 0x0096FF;
inline constexpr cColor LMG_COLOR_YELLOW      = 0xFFCC4C;
inline constexpr cColor LMG_COLOR_LIGHT_GREEN = 0x2ECD72;
#endif //GREEKBOT_LEARNINGGREEK_H