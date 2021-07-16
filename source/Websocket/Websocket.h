#pragma once
#ifndef _GREEKBOT_WEBSOCKET_H_
#define _GREEKBOT_WEBSOCKET_H_

class cWebsocket final {
private:	
	char* m_host = nullptr;
	char* m_path = nullptr;
	char* m_auth = nullptr;
	
public:
	cWebsocket(const char* url, const char* auth = nullptr);
	~cWebsocket();
	
	/* Set events */
	const char* GetHost() const { return m_host ? m_host : "";  }
	const char* GetPath() const { return m_path ? m_path : "/"; }
	const char* GetAuth() const { return m_auth ? m_auth : "";  }
	
	void Run();
};

#endif /* _GREEKBOT_WEBSOCKET_H_ */
