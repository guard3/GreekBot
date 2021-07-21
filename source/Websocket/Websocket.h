#pragma once
#ifndef _GREEKBOT_WEBSOCKET_H_
#define _GREEKBOT_WEBSOCKET_H_
#include "beast.h"

class cWebsocket {
private:
	net::io_context m_ioc;
	ssl::context    m_ctx;
	ws::stream<beast::ssl_stream<beast::tcp_stream>> m_ws;
	
	char* m_host = nullptr;
	char* m_path = nullptr;
	char* m_auth = nullptr;
	
	void ParseStrings(const char* url, const char* auth, size_t auth_size);
	void FreeStrings();
	
public:
	cWebsocket() : m_ctx(ssl::context::tlsv13_client), m_ws(m_ioc, m_ctx) {}
	
	void Read(beast::flat_buffer& buffer) {
		m_ws.read(buffer);
	}
	
	void Write(const net::const_buffer& b, beast::error_code& e) {
		m_ws.write(b, e);
	}
	
	virtual void OnHandshake() {}
	
	/* Set events */
	const char* GetHost() const { return m_host ? m_host : "";  }
	const char* GetPath() const { return m_path ? m_path : "/"; }
	const char* GetAuth() const { return m_auth ? m_auth : "";  }
	
	void Run(const char* url, const char* auth = nullptr, size_t auth_size = 0);
};

#endif /* _GREEKBOT_WEBSOCKET_H_ */
