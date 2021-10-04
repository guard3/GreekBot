#pragma once
#ifndef _GREEKBOT_WEBSOCKET_H_
#define _GREEKBOT_WEBSOCKET_H_
#include "beast.h"

class cWebsocket final {
private:
	net::io_context m_ioc;
	ssl::context    m_ctx;
	ws::stream<beast::ssl_stream<beast::tcp_stream>> m_ws;
	
public:
	explicit cWebsocket(const char* url);
	~cWebsocket() { m_ioc.run(); }
	
	size_t Read(void* buf, size_t size) {
		beast::error_code ec;
		size_t result = m_ws.read_some(net::mutable_buffer(buf, size), ec);
		return ec ? 0 : result;
	}

	size_t Write(const void* buf, size_t size) {
		beast::error_code ec;
		size_t result = m_ws.write(net::const_buffer(buf, size), ec);
		return ec ? 0 : result;
	}

	bool IsOpen() { return m_ws.is_open(); }
	bool IsMessageDone() { return m_ws.is_message_done(); }
	
	void Close(const ws::close_reason& reason = ws::close_code::policy_error) {
		beast::error_code e;
		m_ws.close(reason, e);
	}
};

#endif /* _GREEKBOT_WEBSOCKET_H_ */
