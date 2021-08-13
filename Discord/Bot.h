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

	void RespondToInteraction(const char* interaction_id, const char* token, const std::string& data);
protected:
	virtual void OnInteractionCreate(chInteraction) {}
	
public:
	cBot(const char* token);
	
	const char* GetToken() { return m_token; }

	// TODO: Rate limit
	void AddGuildMemberRole(const cSnowflake& guild_id, const cSnowflake& user_id, const cSnowflake& role_id);
	void RemoveGuildMemberRole(const cSnowflake& guild_id, const cSnowflake& user_id, const cSnowflake& role_id);

	void UpdateGuildMemberRoles(const cSnowflake& guild_id, const cSnowflake& user_id, const std::vector<chSnowflake>& role_ids);

	template<eInteractionCallbackType t>
	void RespondToInteraction(chInteraction interaction, const cInteractionResponse<t>& response) {
		if (interaction)
			RespondToInteraction(interaction->GetId()->ToString(), interaction->GetToken(), response.ToJsonString());
	}
	void EditInteractionResponse(chInteraction interaction, const cInteractionResponse<INTERACTION_CALLBACK_UPDATE_MESSAGE>& response);
	
	void Run();
};


#endif /* _GREEKBOT_BOT_H_ */
