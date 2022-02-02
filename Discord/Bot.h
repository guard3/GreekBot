#pragma once
#ifndef _GREEKBOT_BOT_H_
#define _GREEKBOT_BOT_H_
#include "Interaction.h"
#include "Gateway.h"
#include "Message.h"
#include <vector>

class cBot : public cGateway {
private:
	chUser m_user = nullptr;

	void OnReady(uchUser user) override;

	template<typename T>
	void interaction_response_get_args(const std::vector<T>& v, const T*& ptr, int32_t& sz) {
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
	void interaction_response_get_args(const T(&a)[size], const T*& ptr, int32_t & sz) { ptr = a; sz = size; }
	template<typename T>
	void interaction_response_get_args(std::nullptr_t n, const T*& ptr, int32_t& sz) { ptr = nullptr; sz = -1; }

	bool respond_to_interaction(chInteraction interaction, const char* content, eMessageFlag flags, chActionRow components, int32_t num_components);
	bool edit_interaction_response(chInteraction interaction, const char* content, eMessageFlag flags, chActionRow components, int32_t num_components);

protected:
	using cGateway::OnInteractionCreate;
	using cGateway::OnGuildCreate;
	using cGateway::OnMessageCreate;
	
public:
	explicit cBot(const char* token, eIntent intents) : cGateway(token, intents) {}
	~cBot() { delete m_user; }

	using cGateway::GetToken;

	// TODO: Rate limit
	void AddGuildMemberRole(const cSnowflake& guild_id, const cSnowflake& user_id, const cSnowflake& role_id);
	void RemoveGuildMemberRole(const cSnowflake& guild_id, const cSnowflake& user_id, const cSnowflake& role_id);

	void UpdateGuildMemberRoles(const cSnowflake& guild_id, const cSnowflake& user_id, const std::vector<chSnowflake>& role_ids);

	bool AcknowledgeInteraction(chInteraction interaction);

	template<typename TEmbed, typename TMentions, typename TComponent, typename TAttachment>
	bool RespondToInteraction(chInteraction interaction, const char* content, eMessageFlag flags, const TEmbed& embeds, const TMentions& allowed_mentions, const TComponent& components, const TAttachment& attachments) {
		chActionRow component_args;
		int32_t component_size;
		interaction_response_get_args(components, component_args, component_size);
		return respond_to_interaction(interaction, content, flags, component_args, component_size);
	}

	template<typename TEmbed, typename TMentions, typename TComponent, typename TAttachment>
	bool EditInteractionResponse(chInteraction interaction, const char* content, eMessageFlag flags, const TEmbed& embeds, const TMentions& allowed_mentions, const TComponent& components, const TAttachment& attachments) {
		chActionRow component_args;
		int32_t component_size;
		interaction_response_get_args(components, component_args, component_size);
		return edit_interaction_response(interaction, content, flags, component_args, component_size);
	}
};


#endif /* _GREEKBOT_BOT_H_ */
