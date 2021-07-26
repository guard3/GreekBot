#pragma once
#ifndef _GREEKBOT_BOT_H_
#define _GREEKBOT_BOT_H_
#include "User.h"

class cBot final {
private:
	char  m_http_auth[64] = "Bot ";
	char *m_token         = m_http_auth + 4;
	hUser m_user;
	
public:
	cBot(const char* token);
	
	const char* GetToken() { return m_http_auth + 4; }
	
	bool RegisterSlashCommand(const char* name, const char* description);
	
	void Run();
};


#endif /* _GREEKBOT_BOT_H_ */
