#ifndef GREEKBOT_GREEKBOT_H
#define GREEKBOT_GREEKBOT_H
#include "Bot.h"
#include <unordered_map>
#include <span>

inline const cSnowflake LMG_GUILD_ID = 350234668680871946;
inline const cSnowflake HOLY_EMOJI_ID = 409075809723219969;
inline const cSnowflake HOLY_CHANNEL_ID = 978993330694266920;

/* Specialize std::hash for cSnowflake to use in unordered maps */
template<>
class std::hash<cSnowflake> : std::hash<uint64_t> {
public:
	size_t operator()(const cSnowflake& snowflake) const {
		return std::hash<uint64_t>::operator()(snowflake.ToInt());
	}
};

/* Handling exceptions when they escape the current interaction process function */
struct unhandled_exception_t {
	const char* name;
	std::exception_ptr except;
};
[[noreturn]]
void unhandled_exception(const char*);
/* Helper macros */
#define HANDLER_BEGIN try
#define HANDLER_END catch (...) { unhandled_exception(__func__); }

class cGreekBot final : public cBot {
private:
	std::vector<cRole> m_lmg_roles;
	bool m_bSorted = false;
	std::span<const cRole> get_lmg_roles();

	std::unordered_map<cSnowflake, cGuild> m_guilds;

	cColor get_lmg_member_color(const cPartialMember&);

	std::chrono::steady_clock::time_point m_before = std::chrono::steady_clock::now();

	std::vector<cVoiceState> m_lmg_voice_states;

	cTask<> process_avatar(cAppCmdInteraction&);
	cTask<> process_rank(cAppCmdInteraction&);
	cTask<> process_top(cAppCmdInteraction&);
	cTask<> process_ban(cAppCmdInteraction&);
	cTask<> process_ban_ctx_menu(cAppCmdInteraction&, std::string_view);
	cTask<> process_ban_modal(cModalSubmitInteraction&, std::string_view);
	cTask<> process_ban(cInteraction&, std::uint32_t, const cSnowflake&, std::string_view, std::string_view, std::uint16_t, std::chrono::seconds, std::string_view, std::string_view, std::string_view);
	cTask<> process_unban(cAppCmdInteraction&);
	cTask<> process_unban(cMsgCompInteraction&, cSnowflake);
	cTask<> process_dismiss(cMsgCompInteraction&, cSnowflake);
	cTask<> process_nickname_button(cMsgCompInteraction&, cSnowflake);
	cTask<> process_modal(cModalSubmitInteraction&);
	cTask<> process_role_button(cMsgCompInteraction&, cSnowflake);
	cTask<> process_booster_menu(cMsgCompInteraction&);
	cTask<> process_proficiency_menu(cMsgCompInteraction&);
	cTask<> process_reaction(const cSnowflake&, const cSnowflake&, int64_t, int64_t, std::optional<cMessage>&);
	cTask<> process_starboard_leaderboard(cAppCmdInteraction&);
	cTask<> process_starboard_help(cMsgCompInteraction&);
	cTask<> process_leaderboard_help(cMsgCompInteraction&);
	cTask<> process_timestamp(cAppCmdInteraction&);
	cTask<> process_clear(cAppCmdInteraction&);
	cTask<> process_test(cAppCmdInteraction&);

	cTask<> process_interaction(cAppCmdInteraction&);
	cTask<> process_interaction(cMsgCompInteraction&);
	cTask<> process_interaction(cModalSubmitInteraction&);

	cTask<> process_leaderboard_new_message(cMessage& msg, cPartialMember& member);

	cTask<> OnHeartbeat() override;
	cTask<> OnGuildCreate(cGuild& guild, cGuildCreate& guild_create) override;
	cTask<> OnGuildRoleCreate(cSnowflake& guild_id, cRole& role) override;
	cTask<> OnGuildRoleUpdate(cSnowflake& guild_id, cRole& role) override;
	cTask<> OnGuildRoleDelete(cSnowflake& guild_id, cSnowflake& role_id) override;
	cTask<> OnGuildMemberAdd(cSnowflake& guild_id, cMember& member) override;
	cTask<> OnGuildMemberUpdate(cSnowflake& guild_id, cMemberUpdate& member) override;
	cTask<> OnGuildMemberRemove(cSnowflake& guild_id, cUser& user) override;
	cTask<> OnInteractionCreate(cInteraction&) override;
	cTask<> OnMessageCreate(cMessage& msg, hSnowflake guild_id, hPartialMember member) override;
	cTask<> OnMessageUpdate(cMessageUpdate& msg, hSnowflake guild_id, hPartialMember member) override;
	cTask<> OnMessageDelete(cSnowflake& id, cSnowflake& channel_id, hSnowflake guild_id) override;
	cTask<> OnMessageDeleteBulk(std::span<cSnowflake> ids, cSnowflake& channel_id, hSnowflake guild_id) override;
	cTask<> OnMessageReactionAdd(cSnowflake&, cSnowflake&, cSnowflake&, hSnowflake, hSnowflake, hMember, cEmoji&) override;
	cTask<> OnMessageReactionRemove(cSnowflake&, cSnowflake&, cSnowflake&, hSnowflake, cEmoji&) override;
	cTask<> OnMessageReactionRemoveAll(cSnowflake&, cSnowflake&, hSnowflake) override;
	cTask<> OnMessageReactionRemoveEmoji(cSnowflake&, cSnowflake&, hSnowflake, cEmoji&) override;
	cTask<> OnVoiceStateUpdate(cVoiceState&) override;

public:
	explicit cGreekBot(std::string_view token) : cBot(token, INTENT_GUILD_INTEGRATIONS | INTENT_GUILD_MESSAGES | INTENT_GUILDS | INTENT_GUILD_MEMBERS | INTENT_GUILD_MESSAGE_REACTIONS | INTENT_MESSAGE_CONTENT | INTENT_GUILD_VOICE_STATES) {}
};
#endif /* GREEKBOT_GREEKBOT_H */