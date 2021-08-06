#pragma once
#ifndef _GREEKBOT_WEBSOCKET_H_
#define _GREEKBOT_WEBSOCKET_H_
#include "Utils.h"
#include "beast.h"

class cWebsocket final {
private:
	net::io_context m_ioc;
	ssl::context    m_ctx;
	ws::stream<beast::ssl_stream<beast::tcp_stream>> m_ws;
	
	char* m_host = nullptr;
	char* m_path = nullptr;
	
	std::function<void()> m_eventOnConnect;
	std::function<bool(cWebsocket*, void*, size_t)> m_eventOnMessage;
	
	void Run(char* host, const char* path);
	
public:
	cWebsocket() : m_ctx(ssl::context::tlsv13_client), m_ws(m_ioc, m_ctx) {}
	~cWebsocket() { free(m_host); }
	
	void Read(beast::flat_buffer& buffer) {
		m_ws.read(buffer);
	}
	
	void Write(const net::const_buffer& b, beast::error_code& e) {
		m_ws.write(b, e);
	}
	
	template<typename F>
	cWebsocket& SetOnConnect(F f) {
		m_eventOnConnect = f;
		return *this;
	}
	
	template<typename F>
	cWebsocket& SetOnMessage(F f) {
		m_eventOnMessage = f;
		return *this;
	}
	
	void Run();
	void Run(const char* url);
	
	void Close(const ws::close_reason& reason = ws::close_code::policy_error);
};

#endif /* _GREEKBOT_WEBSOCKET_H_ */
