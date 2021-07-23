#include "Net.h"
#include "beast.h"

bool cNet::GetHttpsRequest(const char *host, const char *path, const char *auth, std::string& response) {
	/* Ensure non null arguments */
	if (!host) host = "";
	if (!path) path = "/";
	
	try {
		/* The IO context for net operations */
		net::io_context ioc;
		
		/* The SSL context */
		ssl::context ctx(ssl::context::tlsv13_client);
		
		/* TODO: Fix certificates and shit */
		//ctx.set_verify_mode(ssl::verify_peer);
		
		/* Create a SSL stream and connect to host */
		beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);
		if (SSL_set_tlsext_host_name(stream.native_handle(), host)) {
			beast::get_lowest_layer(stream).connect(tcp::resolver(ioc).resolve(host, "https"));
			
			/* Perform SSL handshake */
			stream.handshake(ssl::stream_base::client);
			
			/* Make the GET HTTP request */
			http::request<http::string_body> request(http::verb::get, path, 11);
			request.set(http::field::host, host);
			request.set(http::field::user_agent, "GreekBot");
			if (auth)
				request.set(http::field::authorization, auth);
			http::write(stream, request);
			
			/* Read the request response */
			beast::flat_buffer buffer;
			http::response<http::string_body> res;
			http::read(stream, buffer, res);
			/* Shut down the stream */
			beast::error_code e;
			stream.shutdown(e);
			
			/* Return response body */
			response = res.body();
			return true;
		}
	}
	catch (const std::exception& e) {}
	response.clear();
	return false;
}
