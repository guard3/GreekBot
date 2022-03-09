#pragma once
#ifndef _GREEKBOT_BOT_H_
#define _GREEKBOT_BOT_H_
#include "Interaction.h"
#include "Gateway.h"
#include "Message.h"
#include "Embed.h"
#include "Role.h"
#include <vector>
#include "User.h"
#include "Guild.h"
#include "Discord.h"

class cBot : public cGateway {
private:
	chUser m_user = nullptr;

	void OnReady(uchUser user) override;

	template<typename T>
	void resolve_args(const std::vector<T>& v, const T*& ptr, int32_t& sz) {
		if (v.empty()) {
			ptr = nullptr;
			sz = 0;
		}
		else {
			ptr = &v[0];
			sz = v.size();
		}
	}
	template<typename T, size_t size>
	void resolve_args(const T(&a)[size], const T*& ptr, int32_t & sz) { ptr = a; sz = size; }
	template<typename T>
	void resolve_args(std::nullptr_t n, const T*& ptr, int32_t& sz) { ptr = nullptr; sz = -1; }

	enum eIF {
		IF_RESPOND,
		IF_EDIT_OG_MSG,
		IF_FOLLOWUP,
		IF_NUM
	};
	struct sIF_data {
		const char*   http_auth;
		chInteraction interaction; // The interaction we're working with
		const char*   content;
		eMessageFlag  flags;
		chEmbed       embeds;     int32_t num_embeds;     // embeds array
		chActionRow   components; int32_t num_components; // components array

		json::object to_json() const;
	};

	static bool(*ms_interaction_functions[IF_NUM])(const sIF_data*);

	template<typename TEmbed, typename TMentions, typename TComponent, typename TAttachment>
	bool exec_if(eIF func, chInteraction interaction, const char* content, eMessageFlag flags, const TEmbed& embeds, const TMentions& allowed_mentions, const TComponent& components, const TAttachment& attachments) {
		/* Resolve embed array */
		chEmbed embed_args; int32_t embed_size;
		resolve_args(embeds, embed_args, embed_size);
		/* Resolve component array */
		chActionRow component_args; int32_t component_size;
		resolve_args(components, component_args, component_size);
		/* Run appropriate function */
		sIF_data func_data { GetHttpAuthorization(), interaction, content, flags, embed_args, embed_size, component_args, component_size };
		return ms_interaction_functions[func](&func_data);
	}

	template<typename T>
	cSnowflake resolve_snowflake(T&& arg) {
		if constexpr(cHandle::IsHandleType<T>) {
			static_assert(std::is_same_v<cHandle::RemoveHandleCV<T>, cSnowflake>);
			return *arg;
		}
		else return arg;
	}

	std::vector<uchRole> get_guild_roles(const cSnowflake& guild_id);

protected:
	using cGateway::OnInteractionCreate;
	using cGateway::OnGuildCreate;
	using cGateway::OnGuildRoleCreate;
	using cGateway::OnGuildRoleUpdate;
	using cGateway::OnGuildRoleDelete;
	using cGateway::OnMessageCreate;
	
public:
	explicit cBot(const char* token, eIntent intents) : cGateway(token, intents) {}
	~cBot() { delete m_user; }

	using cGateway::GetToken;

	chUser GetUser() const { return m_user; }

	cUser GetUser(const cSnowflake& user_id);
	cMember GetGuildMember(const cSnowflake& guild_id, const cSnowflake& user_id);
	std::vector<cRole> GetGuildRoles(const cSnowflake& guild_id);
	void AddGuildMemberRole(const cSnowflake& guild_id, const cSnowflake& user_id, const cSnowflake& role_id);
	void RemoveGuildMemberRole(const cSnowflake& guild_id, const cSnowflake& user_id, const cSnowflake& role_id);
	void UpdateGuildMemberRoles(const cSnowflake& guild_id, const cSnowflake& user_id, const std::vector<chSnowflake>& role_ids);

	bool AcknowledgeInteraction(chInteraction interaction);
	template<typename... Args>
	bool RespondToInteraction(Args&&... args) { return exec_if(IF_RESPOND, std::forward<Args>(args)...); }
	template<typename... Args>
	bool EditInteractionResponse(Args&&... args) { return exec_if(IF_EDIT_OG_MSG, std::forward<Args>(args)...); }
	template<typename... Args>
	bool SendInteractionFollowupMessage(Args&&... args) { return exec_if(IF_FOLLOWUP, std::forward<Args>(args)...); }
};


#endif /* _GREEKBOT_BOT_H_ */
