#ifndef GREEKBOT_GREEKBOT_H
#define GREEKBOT_GREEKBOT_H
#include "Bot.h"
#include <algorithm>
#include <unordered_map>
#include <span>

/* Define custom message component ids; used for responding to component interactions */
#define CMP_ID_BUTTON_RANK_HELP 0

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
	enum eLmgProficiencyRoleId {
		LMG_PROFICIENCY_NATIVE,
		LMG_PROFICIENCY_BEGINNER,
		LMG_PROFICIENCY_ELEMENTARY,
		LMG_PROFICIENCY_INTERMEDIATE,
		LMG_PROFICIENCY_UPPER_INTERMEDIATE,
		LMG_PROFICIENCY_ADVANCED,
		LMG_PROFICIENCY_FLUENT,
		LMG_PROFICIENCY_NON_LEARNER,
		LMG_NUM_PROFICIENCY_ROLES
	};
	const cSnowflake m_lmg_id = 350234668680871946; // Learning Greek
	const cSnowflake m_lmg_proficiency_roles[8] {
		350483752490631181, // @Native
		351117824300679169, // @Beginner
		351117954974482435, // @Elementary
		350485376109903882, // @Intermediate
		351118486426091521, // @Upper Intermediate
		350485279238258689, // @Advanced
		350483489461895168, // @Fluent
		352001527780474881  // @Non Learner
	};

	std::vector<cRole> m_lmg_roles;
	bool m_bSorted = false;
	std::span<const cRole> get_lmg_roles();

	std::unordered_map<cSnowflake, uhGuild> m_guilds;

	cColor get_lmg_member_color(const cMember&);

	/* Voice */
	std::vector<uint64_t> m_lmg_voice_channels;
	std::vector<std::vector<uint64_t>> m_lmg_users_connected_to_voice;

	cTask<> lmg_update_proficiency_role(chMember member, eLmgProficiencyRoleId proficiency_role) {
		/* The new roles for the member */
		std::vector<chSnowflake> roles;
		roles.reserve(member->GetRoles().size());
		/* Copy all existing roles except proficiency roles */
		const cSnowflake *proficiency_roles_begin = m_lmg_proficiency_roles, *proficiency_roles_end = m_lmg_proficiency_roles + LMG_NUM_PROFICIENCY_ROLES;
		for (auto& id : member->GetRoles()) {
			if (proficiency_roles_end == std::find_if(proficiency_roles_begin, proficiency_roles_end, [&](const cSnowflake& s) { return s == id; }))
				roles.push_back(&id);
		}
		/* Add specified proficiency role */
		roles.push_back(&m_lmg_proficiency_roles[proficiency_role]);
		co_await UpdateGuildMemberRoles(m_lmg_id, member->GetUser()->GetId(), roles);
	}

	cTask<> OnInteraction_SelectMenu(const cInteraction& i) {
		/* Acknowledge interaction */
		co_await RespondToInteraction(i);
		std::string value = i.GetData<INTERACTION_MESSAGE_COMPONENT>().Values[0];
		auto member = i.GetMember();

		if (value == "opt_gr") {
			co_await lmg_update_proficiency_role(member, LMG_PROFICIENCY_NATIVE);
		}
		else if (value == "opt_a1") {
			co_await lmg_update_proficiency_role(member, LMG_PROFICIENCY_BEGINNER);
		}
		else if (value == "opt_a2") {
			co_await lmg_update_proficiency_role(member, LMG_PROFICIENCY_ELEMENTARY);
		}
		else if (value == "opt_b1") {
			co_await lmg_update_proficiency_role(member, LMG_PROFICIENCY_INTERMEDIATE);
		}
		else if (value == "opt_b2") {
			co_await lmg_update_proficiency_role(member, LMG_PROFICIENCY_UPPER_INTERMEDIATE);
		}
		else if (value == "opt_c1") {
			co_await lmg_update_proficiency_role(member, LMG_PROFICIENCY_ADVANCED);
		}
		else if (value == "opt_c2") {
			co_await lmg_update_proficiency_role(member, LMG_PROFICIENCY_FLUENT);
		}
		else if (value == "opt_no") {
			co_await lmg_update_proficiency_role(member, LMG_PROFICIENCY_NON_LEARNER);
		}
		/* Edit original interaction message */
		if (i.GetData<INTERACTION_MESSAGE_COMPONENT>().GetCustomId() == "proficiency_role_menu")
			co_await EditInteractionResponse(i, kw::flags=MESSAGE_FLAG_EPHEMERAL, kw::content="Role assigned!", kw::components=nil);
	}

	cTask<> OnInteraction_avatar(const cInteraction&);
	cTask<> OnInteraction_role(const cInteraction&);
	cTask<> OnInteraction_rank(const cInteraction&);
	cTask<> OnInteraction_top(const cInteraction&);
	cTask<> OnInteraction_button(const cInteraction&);
	cTask<> OnInteraction_prune(const cInteraction&);
	cTask<> OnInteraction_prune_lmg(const cInteraction&);
	cTask<> OnInteraction_ban(const cInteraction&);
	cTask<> OnInteraction_unban(const cInteraction&, const cSnowflake& user_id);
	cTask<> OnInteraction_dismiss(const cInteraction&, const cSnowflake& user_id);

	cTask<> OnGuildCreate(uhGuild guild) override;
	cTask<> OnGuildRoleCreate(cSnowflake& guild_id, cRole& role) override;
	cTask<> OnGuildRoleUpdate(cSnowflake& guild_id, cRole& role) override;
	cTask<> OnGuildRoleDelete(cSnowflake& guild_id, cSnowflake& role_id) override;
	cTask<> OnGuildMemberAdd(cSnowflake& guild_id, cMember& member) override;
	cTask<> OnGuildMemberUpdate(cSnowflake& guild_id, cPartialMember& member) override;
	cTask<> OnGuildMemberRemove(cSnowflake& guild_id, cUser& user) override;
	cTask<> OnInteractionCreate(const cInteraction&) override;
	cTask<> OnMessageCreate(const cMessage& msg) override;

public:
	explicit cGreekBot(std::string_view token) : cBot(token, INTENT_GUILD_INTEGRATIONS | INTENT_GUILD_MESSAGES | INTENT_GUILDS | INTENT_GUILD_MEMBERS) {}
};
#endif /* GREEKBOT_GREEKBOT_H */