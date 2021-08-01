#include "Discord.h"
#include "Net.h"
#include "beast.h"

hGatewayInfo cDiscord::GetGatewayInfo(const char *http_auth) {
	try {
		/* Make HTTP request to the gateway endpoint */
		std::string http_response;
		if (!cNet::GetHttpsRequest(DISCORD_API_HOST, DISCORD_API_GATEWAY_BOT, http_auth, http_response))
			return nullptr;
		
		/* Parse request response as JSON */
		json::monotonic_resource mr;
		json::stream_parser p(&mr);
		p.write(http_response);
		
		/* Construct gateway info object from JSON */
		return new cGatewayInfo(p.release());
	}
	catch (const std::exception&) {
		return nullptr;
	}
}

void cDiscord::RespondToInteraction(const char *http_auth, const char *interaction_id, const char *interaction_token, const std::string &data) {
	try {
		char path[300];
		sprintf(path, DISCORD_API_ENDPOINT "/interactions/%s/%s/callback", interaction_id, interaction_token);
		
		net::io_context ioc;
		ssl::context ctx(ssl::context::tlsv13_client);
		
		/* TODO: Fix certificates and shit */
		//ctx.set_verify_mode(ssl::verify_peer);
		
		beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);
		if (SSL_set_tlsext_host_name(stream.native_handle(), DISCORD_API_HOST)) {
			beast::get_lowest_layer(stream).connect(tcp::resolver(ioc).resolve(DISCORD_API_HOST, "https"));
			
			/* Perform SSL handshake */
			stream.handshake(ssl::stream_base::client);
			
			/* Make the GET HTTP request */
			http::request<http::string_body> request(http::verb::post, path, 11);
			request.set(http::field::host, DISCORD_API_HOST);
			request.set(http::field::user_agent, "GreekBot");
			request.set(http::field::authorization, http_auth);
			request.set(http::field::content_type, "application/json");
			request.set(http::field::content_length, std::to_string(data.length()));
			request.body() = data;
			http::write(stream, request);
			
			/* Read the request response */
			beast::flat_buffer buffer;
			http::response<http::string_body> res;
			http::read(stream, buffer, res);
			
			/* Shut down the stream */
			beast::error_code e;
			stream.shutdown(e);
		}
	}
	catch (const std::exception&) {}
}
