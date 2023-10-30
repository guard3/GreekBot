#ifndef GREEKBOT_GREEKBOT_H
#define GREEKBOT_GREEKBOT_H
#include "Bot.h"
#include <unordered_map>
#include <span>

/* Specialize std::hash for cSnowflake to use in unordered maps */
template<>
class std::hash<cSnowflake> : std::hash<uint64_t> {
public:
	size_t operator()(const cSnowflake& snowflake) const {
		return std::hash<uint64_t>::operator()(snowflake.ToInt());
	}
};

class cGreekBot final : public cBot {
private:
	const cSnowflake m_lmg_id = 350234668680871946; // Learning Greek

	std::vector<cRole> m_lmg_roles;
	bool m_bSorted = false;
	std::span<const cRole> get_lmg_roles();

	std::unordered_map<cSnowflake, uhGuild> m_guilds;

	cColor get_lmg_member_color(const cMember&);

	/* Voice */
	std::vector<uint64_t> m_lmg_voice_channels;
	std::vector<std::vector<uint64_t>> m_lmg_users_connected_to_voice;

	cTask<> OnInteraction_avatar(const cInteraction&);
	cTask<> OnInteraction_rank(const cInteraction&);
	cTask<> OnInteraction_top(const cInteraction&);
	cTask<> OnInteraction_button(const cInteraction&);
	cTask<> OnInteraction_prune(const cInteraction&);
	cTask<> OnInteraction_prune_lmg(const cInteraction&);
	cTask<> OnInteraction_ban(const cInteraction&);
	cTask<> OnInteraction_unban(const cInteraction&, const cSnowflake& user_id);
	cTask<> OnInteraction_dismiss(const cInteraction&, const cSnowflake& user_id);
	cTask<> process_nickname_button(const cInteraction&, const cSnowflake& user_id);
	cTask<> process_modal(const cInteraction&);
	cTask<> process_role_button(const cInteraction&, uint32_t);
	cTask<> process_booster_menu(const cInteraction&);
	cTask<> process_proficiency_menu(const cInteraction&);
	cTask<> process_reaction(const cSnowflake&, const cSnowflake&, int64_t, int64_t, cMessage*);
	cTask<> process_starboard_leaderboard(const cInteraction&);

	cTask<> OnGuildCreate(uhGuild guild) override;
	cTask<> OnGuildRoleCreate(cSnowflake& guild_id, cRole& role) override;
	cTask<> OnGuildRoleUpdate(cSnowflake& guild_id, cRole& role) override;
	cTask<> OnGuildRoleDelete(cSnowflake& guild_id, cSnowflake& role_id) override;
	cTask<> OnGuildMemberAdd(cSnowflake& guild_id, cMember& member) override;
	cTask<> OnGuildMemberUpdate(cSnowflake& guild_id, cPartialMember& member) override;
	cTask<> OnGuildMemberRemove(cSnowflake& guild_id, cUser& user) override;
	cTask<> OnInteractionCreate(const cInteraction&) override;
	cTask<> OnMessageCreate(cMessage& msg, hSnowflake guild_id, hMember member) override;
	cTask<> OnMessageDelete(cSnowflake& id, cSnowflake& channel_id, hSnowflake guild_id) override;
	cTask<> OnMessageDeleteBulk(std::span<cSnowflake> ids, cSnowflake& channel_id, hSnowflake guild_id) override;
	cTask<> OnMessageReactionAdd(cSnowflake&, cSnowflake&, cSnowflake&, hSnowflake, hSnowflake, hMember, cEmoji&) override;
	cTask<> OnMessageReactionRemove(cSnowflake&, cSnowflake&, cSnowflake&, hSnowflake, cEmoji&) override;
	cTask<> OnMessageReactionRemoveAll(cSnowflake&, cSnowflake&, hSnowflake) override;
	cTask<> OnMessageReactionRemoveEmoji(cSnowflake&, cSnowflake&, hSnowflake, cEmoji&) override;

public:
	explicit cGreekBot(std::string_view token) : cBot(token, INTENT_GUILD_INTEGRATIONS | INTENT_GUILD_MESSAGES | INTENT_GUILDS | INTENT_GUILD_MEMBERS | INTENT_GUILD_MESSAGE_REACTIONS) {}
};
#endif /* GREEKBOT_GREEKBOT_H */