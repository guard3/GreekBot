#ifndef GREEKBOT_LEARNINGGREEK_H
#define GREEKBOT_LEARNINGGREEK_H
#include "Base.h"
#include <algorithm>
/* ========== The id of Learning Greek ============================================================================== */
inline constexpr cSnowflake LMG_GUILD_ID = 350234668680871946;
/* ========== Learning Greek channels =============================================================================== */
inline constexpr cSnowflake LMG_CHANNEL_USER_LOG = 469274019565142017;
inline constexpr cSnowflake LMG_CHANNEL_STARBOARD = 978993330694266920;
/* ========== Learning Greek roles ================================================================================== */
inline constexpr cSnowflake LMG_ROLE_ACTIVE_USER          = 352009155864821760; // @Active User
inline constexpr cSnowflake LMG_ROLE_MEMBER               = 350264923806367754; // @Member
inline constexpr cSnowflake LMG_ROLE_ACTIVE_MEMBER        = 352008860904456192; // @Active Member
inline constexpr cSnowflake LMG_ROLE_REALLY_ACTIVE_MEMBER = 353354156783960075; // @Really Active Member
inline constexpr cSnowflake LMG_ROLE_USUAL_SUSPECT        = 410753563543732225; // @Usual Suspect
inline constexpr cSnowflake LMG_ROLE_GREEK_LOVER          = 410754841556549633; // @Greek Lover
inline constexpr cSnowflake LMG_ROLE_PROESTOS             = 414523335595130905; // @Προεστός
inline constexpr cSnowflake LMG_ROLE_EPITIMOS             = 466238555304230913; // @Επίτιμος
inline constexpr cSnowflake LMG_ROLE_ELLINOPOULO          = 608687509291008000; // @Ελληνόπουλο
inline constexpr cSnowflake LMG_ROLE_PALIOS               = 631559446782410752; // @Παλιός
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
/* ========== Emojis ================================================================================================ */
inline constexpr cSnowflake LMG_EMOJI_HOLY = 409075809723219969;
/* ========== Common colors ========================================================================================= */
inline constexpr cColor LMG_COLOR_RED = 0xC43135;
inline constexpr cColor LMG_COLOR_GREEN = 0x248046;
inline constexpr cColor LMG_COLOR_BLUE = 0x0096FF;
#endif //GREEKBOT_LEARNINGGREEK_H