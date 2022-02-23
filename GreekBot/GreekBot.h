#pragma once
#ifndef _GREEKBOT_GREEKBOT_H_
#define _GREEKBOT_GREEKBOT_H_
#include "Bot.h"
#include <mutex>

/* Define custom message component ids; used for responding to component interactions */
#define CMP_ID_BUTTON_RANK_HELP 0

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
	cSnowflake m_lmg_id = 350234668680871946; // Learning Greek
	cSnowflake m_lmg_proficiency_roles[8] {
		350483752490631181, // @Native
		351117824300679169, // @Beginner
		351117954974482435, // @Elementary
		350485376109903882, // @Intermediate
		351118486426091521, // @Upper Intermediate
		350485279238258689, // @Advanced
		350483489461895168, // @Fluent
		352001527780474881  // @Non Learner
	};

	struct {
		uchGuild guild;
		std::vector<cRole> roles;
		std::vector<chRole> sorted_roles;
		std::mutex mutex;
	} m_lmg;

	/* Voice */
	std::vector<uint64_t> m_lmg_voice_channels;
	std::vector<std::vector<uint64_t>> m_lmg_users_connected_to_voice;

	void OnInteraction_avatar(chInteraction interaction);

	void OnInteraction_role(chInteraction interaction);

	void OnInteraction_connect(chInteraction interaction) {
		chMember member = interaction->GetMember();
		//member->
	}

	void lmg_update_proficiency_role(chMember member, eLmgProficiencyRoleId proficiency_role) {
		/* The new roles for the member */
		std::vector<chSnowflake> roles;
		roles.reserve(member->Roles.size());
		/* Copy all existing roles except proficiency roles */
		cSnowflake *proficiency_roles_begin = m_lmg_proficiency_roles, *proficiency_roles_end = m_lmg_proficiency_roles + LMG_NUM_PROFICIENCY_ROLES;
		for (chSnowflake p : member->Roles) {
			if (proficiency_roles_end == std::find_if(proficiency_roles_begin, proficiency_roles_end, [&](const cSnowflake& s) { return s.ToInt() == p->ToInt(); }))
				roles.push_back(p);
		}
		/* Add specified proficiency role */
		roles.push_back(&m_lmg_proficiency_roles[proficiency_role]);
		UpdateGuildMemberRoles(m_lmg_id, *member->GetUser()->GetId(), roles);
	}

	void OnInteraction_SelectMenu(chInteraction interaction) {
		/* Acknowledge interaction */
		AcknowledgeInteraction(interaction);
		const char* value = interaction->GetData<INTERACTION_MESSAGE_COMPONENT>()->Values[0];
		auto member = interaction->GetMember();

		if (0 == strcmp(value, "opt_gr")) {
			lmg_update_proficiency_role(member, LMG_PROFICIENCY_NATIVE);
		}
		else if (0 == strcmp(value, "opt_a1")) {
			lmg_update_proficiency_role(member, LMG_PROFICIENCY_BEGINNER);
		}
		else if (0 == strcmp(value, "opt_a2")) {
			lmg_update_proficiency_role(member, LMG_PROFICIENCY_ELEMENTARY);
		}
		else if (0 == strcmp(value, "opt_b1")) {
			lmg_update_proficiency_role(member, LMG_PROFICIENCY_INTERMEDIATE);
		}
		else if (0 == strcmp(value, "opt_b2")) {
			lmg_update_proficiency_role(member, LMG_PROFICIENCY_UPPER_INTERMEDIATE);
		}
		else if (0 == strcmp(value, "opt_c1")) {
			lmg_update_proficiency_role(member, LMG_PROFICIENCY_ADVANCED);
		}
		else if (0 == strcmp(value, "opt_c2")) {
			lmg_update_proficiency_role(member, LMG_PROFICIENCY_FLUENT);
		}
		else if (0 == strcmp(value, "opt_no")) {
			lmg_update_proficiency_role(member, LMG_PROFICIENCY_NON_LEARNER);
		}
		/* Edit original interaction message */
		EditInteractionResponse(interaction, "Role assigned!", MESSAGE_FLAG_EPHEMERAL, nullptr, nullptr, std::vector<cActionRow>(), nullptr);
	}

	void OnInteraction_rank(chInteraction interaction);
	void OnInteraction_top(chInteraction interaction);
	void OnInteraction_button(chInteraction interaction);

	void OnGuildCreate(uhGuild guild) override;
	void OnGuildRoleCreate(chSnowflake guild_id, hRole role) override;
	void OnGuildRoleUpdate(chSnowflake guild_id, hRole role) override;
	void OnGuildRoleDelete(chSnowflake guild_id, chSnowflake role_id) override;
	void OnInteractionCreate(chInteraction interaction) override;
	void OnMessageCreate(chMessage msg) override;

public:
	explicit cGreekBot(const char* token) : cBot(token, INTENT_GUILD_INTEGRATIONS | INTENT_GUILD_MESSAGES | INTENT_GUILDS) {}
};
#endif /* _GREEKBOT_GREEKBOT_H_ */