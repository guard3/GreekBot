#pragma once
#ifndef _GREEKBOT_BOT_H_
#define _GREEKBOT_BOT_H_
#include "User.h"
#include "Interaction.h"

class cBot final {
private:
	char  m_http_auth[64] = "Bot ";
	char *m_token         = m_http_auth + 4;
	uchUser m_user;
	
	void OnInteractionCreate(uchInteraction);
	
public:
	cBot(const char* token);
	
	const char* GetToken() { return m_token; }
	
	void Run();
};


#endif /* _GREEKBOT_BOT_H_ */
