#include "Net.h"
#include "beast.h"
#include <iostream>

static bool https_request(http::verb method, const char* host, const char* path, const char* auth, const char* content_type, std::string content) {
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

			std::cout << "PATCH: " << res.body() << std::endl;
			/* Shut down the stream */
			beast::error_code e;
			stream.shutdown(e);
			return true;
		}
	}
	catch (...) {}
	return false;
}

unsigned int cNet::GetHttpsRequest(const char *host, const char *path, const char *auth, std::string& response) {
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
		/* Create an SSL stream */
		beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);
		if (SSL_set_tlsext_host_name(stream.native_handle(), host)) {
			/* Connect to host */
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
			response = std::move(res.body());
			return res.result_int();
		}
	}
	catch (...) {
		response.clear();
	}
	return 0;
}

bool cNet::PutHttpsRequest(const char *host, const char *path, const char *auth) {
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
			http::request<http::string_body> request(http::verb::put, path, 11);
			request.set(http::field::host, host);
			request.set(http::field::user_agent, "GreekBot");
			if (auth)
				request.set(http::field::authorization, auth);
			request.set(http::field::content_length, "0");
			http::write(stream, request);

			/* Read the request response */
			beast::flat_buffer buffer;
			http::response<http::string_body> res;
			http::read(stream, buffer, res);

			std::cout << res.body() << std::endl;
			/* Shut down the stream */
			beast::error_code e;
			stream.shutdown(e);
			return true;
		}
	}
	catch (...) {}
	return false;
}

bool cNet::DeleteHttpsRequest(const char* host, const char* path, const char* auth) {
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
			http::request<http::string_body> request(http::verb::delete_, path, 11);
			request.set(http::field::host, host);
			request.set(http::field::user_agent, "GreekBot");
			if (auth)
				request.set(http::field::authorization, auth);
			request.set(http::field::content_length, "0");
			http::write(stream, request);

			/* Read the request response */
			beast::flat_buffer buffer;
			http::response<http::string_body> res;
			http::read(stream, buffer, res);

			std::cout << res.body() << std::endl;
			/* Shut down the stream */
			beast::error_code e;
			stream.shutdown(e);
			return true;
		}
	}
	catch (...) {}
	return false;
}

bool cNet::PostHttpsRequest(const char *host, const char *path, const char *auth, const json::object& obj) {
	return https_request(http::verb::post, host, path, auth, "application/json", (std::stringstream() << obj).str());
}
bool cNet::PostHttpsRequest(const char *host, const char *path, const char *auth, json::object&& obj) {
	return https_request(http::verb::post, host, path, auth, "application/json", (std::stringstream() << obj).str());
}

bool cNet::PatchHttpsRequest(const char *host, const char *path, const char *auth, const std::string &data) {
	return https_request(http::verb::patch, host, path, auth, nullptr, data);
}
bool cNet::PatchHttpsRequest(const char *host, const char *path, const char *auth, const json::object &obj) {
	return https_request(http::verb::patch, host, path, auth, "application/json", (std::stringstream() << obj).str());
}