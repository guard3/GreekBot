#pragma once
#ifndef _GREEKBOT_BOT_H_
#define _GREEKBOT_BOT_H_

class cBot final {
private:
	char m_http_auth[64] = "Bot ";
	
public:
	cBot(const char* token);
	
	const char* GetToken() { return m_http_auth + 4; }
	
	void Run();
};


#endif /* _GREEKBOT_BOT_H_ */
