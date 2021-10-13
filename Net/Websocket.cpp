#include "Websocket.h"

cWebsocket::cWebsocket(const char *url) : m_ctx(ssl::context::tlsv13_client), m_ws(m_ioc, m_ctx) {
	/* Parse url */
	char *host;
	const char *path;
	size_t host_len;
	if (url) {
		const char *h, *p;
		if ((h = strstr(url, "://"))) {
			h += 3;

			if ((p = strchr(h, '/'))) {
				host_len = p - h;
				if ((host = reinterpret_cast<char*>(malloc(host_len + 7)))) {
					memcpy(host, h, host_len);
					host[host_len] = '\0';
					path = p;
					goto LABEL_URL_PARSED;
				}
			}
			else {
				host_len = strlen(h);
				if ((host = reinterpret_cast<char*>(malloc(host_len + 7)))) {
					strcpy(host, h);
					path = "/";
					goto LABEL_URL_PARSED;
				}
			}
		}
	}
	return;

	LABEL_URL_PARSED:
	try {
		/* Connect websocket stream to host */
		beast::get_lowest_layer(m_ws).expires_after(std::chrono::seconds(30));
		auto ep = beast::get_lowest_layer(m_ws).connect(tcp::resolver(m_ioc).resolve(host, "https"));
		/* Set SNI Hostname */
		if (SSL_set_tlsext_host_name(m_ws.next_layer().native_handle(), host)) {
			/* Append resolved port number to host name */
			sprintf(host + host_len, ":%d", ep.port() & 0xFFFF);
			/* Set HTTP header fields for the handshake */
			m_ws.set_option(ws::stream_base::decorator([&](ws::request_type& r) {
				r.set(http::field::host, host);
				r.set(http::field::user_agent, "GreekBot");
			}));
			/* Perform SSL handshake */
			m_ws.next_layer().handshake(ssl::stream_base::client);
			beast::get_lowest_layer(m_ws).expires_never();
			m_ws.set_option(ws::stream_base::timeout::suggested(beast::role_type::client));
			/* Perform websocket handshake */
			m_ws.handshake(host, path);
		}
	}
	catch (...) {}

	free(host);
}
