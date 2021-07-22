#include "Websocket.h"
#include "beast.h"
#include <iostream>

void cWebsocket::Run() {
	char s[] = "\0\0\0\0\0\0";
	Run(m_host ? m_host : s, m_path ? m_path : "/");
}

void cWebsocket::Run(const char* url) {
	free(m_host);
	m_host = m_path = nullptr;
	
	if (url) {
		const char *host, *path;
		if ((host = strstr(url, "://"))) {
			host += 3;
			
			if ((path = strchr(host, '/'))) {
				size_t host_len = path - host;
				size_t path_len = strlen(path);
				if ((m_host = reinterpret_cast<char*>(malloc(host_len + path_len + 8)))) {
					memcpy(m_host, host, host_len);
					m_host[host_len] = '\0';
					m_path = m_host + host_len + 7;
					strcpy(m_path, path);
				}
			}
			else if ((m_host = reinterpret_cast<char*>(malloc(strlen(host) + 7))))
				strcpy(m_host, host);
		}
	}
	
	Run();
}

void cWebsocket::Run(char* host, const char* path) {
	try {
		/* Connect websocket stream to host */
		beast::get_lowest_layer(m_ws).expires_after(std::chrono::seconds(30));
		auto ep = beast::get_lowest_layer(m_ws).connect(tcp::resolver(m_ioc).resolve(host, "https"));
		
		/* Set SNI Hostname */
		if (!SSL_set_tlsext_host_name(m_ws.next_layer().native_handle(), host))
			throw beast::system_error(beast::error_code(static_cast<int>(ERR_get_error()), net::error::get_ssl_category()), "Failed to set SNI Hostname");
		
		/* Append resolved port number to host name */
		sprintf(host + strlen(host), ":%d", ep.port() & 0xFFFF);
		
		/* Set HTTP header fields for the handshake */
		m_ws.set_option(ws::stream_base::decorator([&](ws::request_type& r) {
			r.set(http::field::host, host);
			r.set(http::field::user_agent, "GreekBot");
		}));
		
		/* Perform SSL handshake */
		m_ws.next_layer().handshake(ssl::stream_base::client);
		beast::get_lowest_layer(m_ws).expires_never();
		//ws.set_option(ws::stream_base::timeout::suggested(beast::role_type::client));
		
		m_ws.handshake(host, path);
		
		m_eventOnConnect();
		
		m_ioc.run();
	}
	catch (const std::exception&) {}
}
