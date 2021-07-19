#include "Websocket.h"
#include "beast.h"
#include <iostream>

void cWebsocket::ParseStrings(const char* url, const char* auth, size_t auth_size) {
	/* Store HTTP authorization string */
	m_auth = nullptr;
	if (auth) {
		size_t size = auth_size ? auth_size : strlen(auth) + 1;
		if ((m_auth = reinterpret_cast<char*>(malloc(size))))
			memcpy(m_auth, auth, size);
	}
	
	/* Parse url */
	m_host = m_path = nullptr;
	if (url) {
		const char *host, *path;
		if ((host = strstr(url, "://"))) {
			host += 3;
			
			if ((path = strchr(host, '/'))) {
				size_t host_len = path - host;
				size_t path_len = strlen(path);
				if ((m_path = reinterpret_cast<char*>(malloc(host_len + path_len + 8)))) {
					strcpy(m_path, path);
					m_host = m_path + path_len + 1;
					memcpy(m_host, host, host_len);
					m_host[host_len] = '\0';
				}
			}
			else {
				if ((m_path = reinterpret_cast<char*>(malloc(strlen(host) + 9)))) {
					m_path[0] = '/';
					m_path[1] = '\0';
					m_host = m_path + 2;
					strcpy(m_host, host);
				}
			}
		}
	}
}

void cWebsocket::FreeStrings() {
	free(m_path);
	free(m_auth);
	m_host = m_path = m_auth = nullptr;
}

void cWebsocket::Run(const char* url, const char* auth, size_t auth_size) {
	ParseStrings(url, auth, auth_size);
	
	try {
		/* Connect websocket stream to host */
		beast::get_lowest_layer(m_ws).expires_after(std::chrono::seconds(30));
		auto ep = beast::get_lowest_layer(m_ws).connect(tcp::resolver(m_ioc).resolve(m_host, "https"));
		
		/* Set SNI Hostname */
		if (!SSL_set_tlsext_host_name(m_ws.next_layer().native_handle(), m_host))
			throw beast::system_error(beast::error_code(static_cast<int>(ERR_get_error()), net::error::get_ssl_category()), "Failed to set SNI Hostname");
		
		/* Append resolved port number to host name */
		sprintf(m_host + strlen(m_host), ":%d", ep.port() & 0xFFFF);
		
		/* Set HTTP header fields for the handshake */
		m_ws.set_option(ws::stream_base::decorator([&](ws::request_type& r) {
			r.set(http::field::host, m_host);
			r.set(http::field::user_agent, "GreekBot");
			if (m_auth)
				r.set(http::field::authorization, m_auth);
		}));
		
		/* Perform SSL handshake */
		m_ws.next_layer().handshake(ssl::stream_base::client);
		beast::get_lowest_layer(m_ws).expires_never();
		//ws.set_option(ws::stream_base::timeout::suggested(beast::role_type::client));
		
		m_ws.handshake(m_host, m_path);
		
		OnHandshake();
		
		m_ioc.run();
	}
	catch (const std::exception&) {}
	
	FreeStrings();
}
