#include "beast.h"
#include <iostream>
#include <memory>
#include <chrono>
#include "Discord.h"

class cWebSocketSession final : public std::enable_shared_from_this<cWebSocketSession> {
private:
	tcp::resolver m_resolver;         // The ip address resolver
	ws::stream<beast::ssl_stream<beast::tcp_stream>> m_ws_stream; // The websocket stream
	beast::flat_buffer m_buffer;
	char* m_host = nullptr;
	char* m_authorization = nullptr;
	size_t m_host_len = 0;
	
	void OnResolve(beast::error_code e, tcp::resolver::results_type results) {
		if (e)
			std::cout << "FAIL OnResolve: " << e.message() << std::endl;
		
		/* Connect to tcp stream */
		beast::get_lowest_layer(m_ws_stream).expires_after(std::chrono::seconds(30));
		beast::get_lowest_layer(m_ws_stream).async_connect(results, beast::bind_front_handler(&cWebSocketSession::OnConnect, shared_from_this()));
	}
	
	void OnConnect(beast::error_code e, tcp::resolver::results_type::endpoint_type ep) {
		if (e)
			std::cout << "FAIL OnConnect: " << e.message() << std::endl;
		
		/* Add port number to host */
		sprintf(m_host + m_host_len, ":%i", ep.port() & 0xFFFF);
		
		/* Set SNI Hostname */
		if (!SSL_set_tlsext_host_name(m_ws_stream.next_layer().native_handle(), m_host)) {
			std::cout << "FAIL OnConnect: " << beast::error_code(static_cast<int>(ERR_get_error()), net::error::get_ssl_category()).message() << std::endl;
		}
		
		/* Perform SSL Handshake */
		beast::get_lowest_layer(m_ws_stream).expires_after(std::chrono::seconds(30));
		m_ws_stream.next_layer().async_handshake(ssl::stream_base::client, beast::bind_front_handler(&cWebSocketSession::OnSSLHandshake, shared_from_this()));
	}
	
	void OnSSLHandshake(beast::error_code e) {
		if (e)
			std::cout << "FAIL OnSSLHandshake: " << e.message() << std::endl;
		
		/* Set recommended timeout for client */
		beast::get_lowest_layer(m_ws_stream).expires_never();
		m_ws_stream.set_option(ws::stream_base::timeout::suggested(beast::role_type::client));
		
		/* Set decorators */
		m_ws_stream.set_option(ws::stream_base::decorator([this](ws::request_type& r) {
			r.set(http::field::user_agent, "GreekBot");
			r.set(http::field::authorization, m_authorization);
		}));
		
		/* Perform WebSocket Handshake */
		m_ws_stream.async_handshake(m_host, "/", beast::bind_front_handler(&cWebSocketSession::OnHandshake, shared_from_this()));
	}
	
	void OnHandshake(beast::error_code e) {
		if (e)
			std::cout << "FAIL OnHandshake: " << e.message() << std::endl;
		
		//for (;;) {
			m_ws_stream.async_read(m_buffer, beast::bind_front_handler(&cWebSocketSession::OnRead, shared_from_this()));
		//}
	}
	
	void OnRead(beast::error_code e, size_t size) {
		std::cout << (char*)m_buffer.data().data() << std::endl;
		
		m_buffer.consume(size);
	}
	
public:
	/* Initialize resolver and stream as async */
	explicit cWebSocketSession(net::io_context& ioc, ssl::context& ctx) : m_resolver(net::make_strand(ioc)), m_ws_stream(net::make_strand(ioc), ctx) {}
	
	~cWebSocketSession() {
		free(m_authorization);
	}
	
	void Run(const char* host, const char* port, const char* token) {
		size_t hostlen = strlen(host);
		
		void* temp;
		if ((temp = malloc(hostlen + 71))) {
			/* Save host and authorization info */
			m_authorization = reinterpret_cast<char*>(temp);
			sprintf(m_authorization, "Bot %.59s", token);
			m_host = m_authorization + 64;
			strcpy(m_host, host);
			
			/* Resolve domain name */
			m_resolver.async_resolve(host, port, beast::bind_front_handler(&cWebSocketSession::OnResolve, shared_from_this()));
		}
		
		m_host_len = hostlen;
	}
	
};


int main() {
	
	json::value v;
	cDiscord::GetGateway(nullptr, v);
	
	/*
	net::io_context ioc;
	ssl::context ctx { ssl::context::tlsv13_client };
	
	std::make_shared<cWebSocketSession>(ioc, ctx)->Run("gateway.discord.gg", "https", "boo");
	
	ioc.run();
	 */
	
	std::cout << "HELLO";
	return 0;
}
