#pragma once
#ifndef _GREEKBOT_BOT_H_
#define _GREEKBOT_BOT_H_
#include "User.h"
#include "Interaction.h"
#include "InteractionResponse.h"

class cBot {
private:
	char  m_http_auth[64] = "Bot ";
	char *m_token         = m_http_auth + 4;
	uchUser m_user;

	void respond_to_interaction(chInteraction interaction, json::object&& obj);
	void edit_interaction_response(chInteraction interaction, json::object&& obj);

	template<eInteractionCallbackType t>
	void respond_to_interaction(chInteraction interaction, const cInteractionResponse<t>& response) { respond_to_interaction(interaction, response.ToJson()); }
	template<eInteractionCallbackType t>
	void respond_to_interaction(chInteraction interaction, cInteractionResponse<t>&& response) { respond_to_interaction(interaction, response.ToJson()); }
	void edit_interaction_response(chInteraction interaction, const cInteractionResponse<INTERACTION_CALLBACK_UPDATE_MESSAGE>& response) { edit_interaction_response(interaction, response.ToJson()); }
	void edit_interaction_response(chInteraction interaction, cInteractionResponse<INTERACTION_CALLBACK_UPDATE_MESSAGE>&& response) { edit_interaction_response(interaction, response.ToJson()); }

protected:
	virtual void OnInteractionCreate(chInteraction) {}
	
public:
	cBot(const char* token);
	
	const char* GetToken() { return m_token; }

	// TODO: Rate limit
	void AddGuildMemberRole(const cSnowflake& guild_id, const cSnowflake& user_id, const cSnowflake& role_id);
	void RemoveGuildMemberRole(const cSnowflake& guild_id, const cSnowflake& user_id, const cSnowflake& role_id);

	void UpdateGuildMemberRoles(const cSnowflake& guild_id, const cSnowflake& user_id, const std::vector<chSnowflake>& role_ids);

	template<typename T>
	void RespondToInteraction(chInteraction interaction, T&& response) { respond_to_interaction(interaction, std::forward<T>(response)); }
	template<eInteractionCallbackType t, typename... Args>
	void RespondToInteraction(chInteraction interaction, Args&&... args) { respond_to_interaction(interaction, cInteractionResponse<t>(std::forward<Args>(args)...)); }


	template<typename T>
	void EditInteractionResponse(chInteraction interaction, T&& response) { edit_interaction_response(interaction, std::forward<T>(response)); }
	template<typename... Args>
	void EditInteractionResponse(chInteraction interaction, Args... args) { edit_interaction_response(interaction, cInteractionResponse<INTERACTION_CALLBACK_UPDATE_MESSAGE>(std::forward<Args>(args)...)); }
	
	void Run();
};


#endif /* _GREEKBOT_BOT_H_ */
