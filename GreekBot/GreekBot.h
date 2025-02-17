#ifndef GREEKBOT_GREEKBOT_H
#define GREEKBOT_GREEKBOT_H
#include "Bot.h"
#include "LearningGreek.h"
#include "SQLite.h"
#include <unordered_map>
#include <span>

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

	/* TODO: put these in cDatabase after refactoring */
	static cTask<sqlite::connection> BorrowDatabase();
	static cTask<> ReturnDatabase(sqlite::connection);

	cTask<> process_avatar(cAppCmdInteraction&);
	cTask<> process_rank(cAppCmdInteraction&);
	cTask<> process_top(cAppCmdInteraction&);
	cTask<> process_ban(cAppCmdInteraction&);
	cTask<> process_ban_ctx_menu(cAppCmdInteraction&, std::string_view);
	cTask<> process_ban_modal(cModalSubmitInteraction&, std::string_view);
	cTask<> process_ban(cInteraction&, std::uint32_t, const cSnowflake&, std::string_view, std::string_view, std::uint16_t, std::chrono::seconds, std::string_view, std::string_view, std::string_view);
	cTask<> process_unban(cInteraction&, cSnowflake = {});
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
	cTask<> process_warn(cAppCmdInteraction&);
	cTask<> process_warn(cModalSubmitInteraction&, std::string_view);
	cTask<> process_warn_impl(cInteraction&, const cSnowflake& user_id, std::string_view username, std::string_view avatar, std::uint16_t discriminator, std::string_view reason);
	cTask<> process_infractions(cAppCmdInteraction&);
	cTask<> process_infractions_button(cMsgCompInteraction&, cSnowflake);
	cTask<> process_infractions_remove(cMsgCompInteraction&, std::string_view);
	cTask<> process_timeout_remove(cMsgCompInteraction&, cSnowflake);
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