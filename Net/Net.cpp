#include "Net.h"
#include "beast.h"
#include <iostream>

static int https_request(http::verb method, const char* host, const char* path, const char* auth, const char* content_type, std::string content, std::string& response) {
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
			http::request<http::string_body> request(method, path, 11);
			request.set(http::field::host, host);
			request.set(http::field::user_agent, "GreekBot");
			if (auth)
				request.set(http::field::authorization, auth);
			if (content_type)
				request.set(http::field::content_type, content_type);
			request.set(http::field::content_length, std::to_string(content.length()));
			request.body() = std::move(content);
			http::write(stream, request);

			/* Read the request response */
			beast::flat_buffer buffer;
			http::response<http::string_body> res;
			http::read(stream, buffer, res);
			response = std::move(res.body());

			/* Shut down the stream */
			beast::error_code e;
			stream.shutdown(e);
			return res.result_int();
		}
	}
	catch (...) {}
	return -1;
}

int cNet::GetHttpsRequest(const char* host, const char* path, const char* auth, std::string& response) {
	return https_request(http::verb::get, host, path, auth, nullptr, std::string(), response);
}

int cNet::PostHttpsRequest(const char *host, const char *path, const char *auth, const json::object& obj) {
	std::string response;
	return https_request(http::verb::post, host, path, auth, "application/json", (std::stringstream() << obj).str(), response);
}

int cNet::PatchHttpsRequest(const char *host, const char *path, const char *auth, const json::object &obj) {
	std::string response;
	return https_request(http::verb::patch, host, path, auth, "application/json", (std::stringstream() << obj).str(), response);
}

int cNet::PutHttpsRequest(const char *host, const char *path, const char *auth) {
	std::string response;
	return https_request(http::verb::put, host, path, auth, nullptr, std::string(), response);
}

int cNet::DeleteHttpsRequest(const char* host, const char* path, const char* auth) {
	std::string response;
	return https_request(http::verb::delete_, host, path, auth, nullptr, std::string(), response);
}