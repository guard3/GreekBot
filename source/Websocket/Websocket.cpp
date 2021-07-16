#include "Websocket.h"
#include "beast.h"
#include <iostream>

cWebsocket::cWebsocket(const char* url, const char* auth) {
	/* Parse url */
	if (url) {
		const char *host, *path;
		if ((host = strstr(url, "://"))) {
			host += 3;
			
			/* Calculate string lengths */
			size_t len = static_cast<bool>(auth) * 64;
			size_t host_len, path_len;
			if ((path = strchr(host, '/'))) {
				host_len = path - host;
				path_len = strlen(path);
			}
			else {
				host_len = strlen(host);
				path_len = 1;
			}
			len += host_len + path_len + 2;
			
			/* Copy strings */
			if ((m_path = reinterpret_cast<char*>(malloc(len)))) {
				if (path) {
					strcpy(m_path, path);
					m_host = m_path + path_len + 1;
					strncpy(m_host, host, host_len);
					m_host[host_len] = '\0';
				}
				else {
					m_path[0] = '/';
					m_path[1] = '\0';
					m_host = m_path + path_len + 1;
					strcpy(m_host, host);
				}
				if (auth) {
					m_auth = m_host + host_len + 1;
					strncpy(m_auth, auth, 63);
					m_auth[63] = '\0';
				}
			}
		}
	}
}

void cWebsocket::Run() {
	try {
		/* The necessary contexts */
		net::io_context ioc;
		ssl::context ctx(ssl::context::tlsv13_client);
		
		ws::stream<beast::ssl_stream<beast::tcp_stream>> ws(ioc, ctx);
		beast::get_lowest_layer(ws).expires_after(std::chrono::seconds(30));
		auto ep = beast::get_lowest_layer(ws).connect(tcp::resolver(ioc).resolve(m_host, "https"));
		
		if (!SSL_set_tlsext_host_name(ws.next_layer().native_handle(), m_host))
			throw beast::system_error(beast::error_code(static_cast<int>(ERR_get_error()), net::error::get_ssl_category()), "Failed to set SNI Hostname");
		
		std::string host(m_host);
		host += ':' + std::to_string(ep.port());
		
		ws.set_option(ws::stream_base::decorator([&](ws::request_type& r) {
			r.set(http::field::host, host);
			r.set(http::field::user_agent, "GreekBot");
			if (m_auth)
				r.set(http::field::authorization, m_auth);
		}));
		
		ws.next_layer().handshake(ssl::stream_base::client);
		beast::get_lowest_layer(ws).expires_never();
		ws.set_option(ws::stream_base::timeout::suggested(beast::role_type::client));
		
		ws.handshake(host, m_path);
		
		beast::flat_buffer buffer;
		
		ws.read(buffer);
		
		std::cout << (char*)buffer.data().data();
		
		ioc.run();
	}
	catch (const std::exception&) {}
}

cWebsocket::~cWebsocket() {
	free(m_path);
}
