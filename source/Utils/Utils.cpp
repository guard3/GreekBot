#include "Utils.h"
#include "beast.h"

/* Random engine stuff */
static std::random_device s_rd;
static std::mt19937 s_gen(s_rd());
static std::uniform_real_distribution<float> s_dis(0.0f, 0.1f);


float cUtils::Random() {
	return s_dis(s_gen);
}

void cUtils::Print(FILE* f, const char* comment, const char* fmt, va_list args) {
	fputs(comment, f);
	vfprintf(f, fmt, args);
	fputc('\n', f);
}

void cUtils::PrintErr(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	cUtils::Print(stderr, "[ERR] ", fmt, args);
	va_end(args);
}

void cUtils::PrintLog(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	cUtils::Print(stdout, "[LOG] ", fmt, args);
	va_end(args);
}

std::string cUtils::GetHttpsRequest(const char* host, const char* path, const char* auth) {
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
		if(!SSL_set_tlsext_host_name(stream.native_handle(), host))
			throw beast::system_error(beast::error_code(static_cast<int>(ERR_get_error()), net::error::get_ssl_category()));
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
		http::response<http::string_body> response;
		http::read(stream, buffer, response);
		
		/* Shut down the stream */
		beast::error_code e;
		stream.shutdown(e);
		if (e == net::error::eof || e == ssl::error::stream_truncated)
			e = {};
		if (e)
			throw beast::system_error(e);
		
		/* Return response body */
		return response.body();
	}
	catch (const std::exception& e) {
		fprintf(stderr, "Error in cUtils::GetHttpsRequest()\n%s\n", e.what());
		return std::string();
	}
}
