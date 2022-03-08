#include "Net.h"
#include "beast.h"
#include "json.h"
#include "Discord.h"

#include <tuple>

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
	return https_request(http::verb::post, host, path, auth, "application/json", json::serialize(obj), response);
}

int cNet::PatchHttpsRequest(const char *host, const char *path, const char *auth, const json::object &obj) {
	std::string response;
	return https_request(http::verb::patch, host, path, auth, "application/json", json::serialize(obj), response);
}

int cNet::PutHttpsRequest(const char *host, const char *path, const char *auth) {
	std::string response;
	return https_request(http::verb::put, host, path, auth, nullptr, std::string(), response);
}

int cNet::DeleteHttpsRequest(const char* host, const char* path, const char* auth) {
	std::string response;
	return https_request(http::verb::delete_, host, path, auth, nullptr, std::string(), response);
}

static std::tuple<beast::http::status, json::value> http_request(http::verb method, const std::string& path, const std::string& auth, const json::object* content) {
	/* The IO context for net operations */
	net::io_context ioc;
	/* The SSL context */
	ssl::context ctx(ssl::context::tlsv13_client);
	/* TODO: Fix certificates and shit */
	//ctx.set_verify_mode(ssl::verify_peer);
	/* Create an SSL stream and connect to host */
	beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);
	beast::get_lowest_layer(stream).connect(tcp::resolver(ioc).resolve(DISCORD_API_HOST, "https"));
	/* Perform SSL handshake */
	stream.handshake(ssl::stream_base::client);
	/* Prepare the HTTP request */
	http::request<http::string_body> request(method, path, 11);
	request.set(http::field::host, DISCORD_API_HOST);
	request.set(http::field::user_agent, "GreekBot");
	request.set(http::field::authorization, auth);
	if (content) {
		std::string str = json::serialize(*content);
		request.set(beast::http::field::content_type, "application/json");
		request.set(beast::http::field::content_length, std::to_string(str.length()));
		request.body() = std::move(str);
	}
	else request.set(beast::http::field::content_length, "0");
	http::write(stream, request);
	/* Read the request response */
	beast::flat_buffer buffer;
	http::response<http::string_body> res;
	http::read(stream, buffer, res);
	/* Shut down the stream */
	beast::error_code e;
	stream.shutdown(e);
	auto f = res.find(beast::http::field::content_type);
	return { res.result(), f == res.end() ? json::value() : f->value() == "application/json" ? json::parse(res.body()) : json::object() };
}

xSystemError::xSystemError(const json::object& o) : std::runtime_error([](const json::value* v) -> const char* {
	if (const json::string* s; v && (s = v->if_string()))
		return s->c_str();
	return "An error occurred";
}(o.if_contains("message"))), m_code(0) {
	if (auto p = o.if_contains("code"))
		if (auto i = p->if_int64())
			m_code = *i;
}

xSystemError::xSystemError(const json::value& v) : xSystemError(v.as_object()) {}

xSystemError::xSystemError(const boost::system::system_error& e) : xSystemError(e.what(), e.code().value()) {}

xDiscordError::xDiscordError(const json::object& o) : xSystemError(o) {
	try {
		m_errors = json::serialize(o.at("errors"));
	}
	catch (...) {}
}

xDiscordError::xDiscordError(const json::value& v) : xDiscordError(v.as_object()) {}

static json::value discord_http_request(beast::http::verb method, const std::string& path, const std::string& auth, const json::object* content) {
	try {
		/* Perform the http request */
		beast::http::status status;
		json::value result;
		std::tie(status, result) = http_request(method, path, auth, content);
		/* If request was successful, return received json value */
		if (beast::http::to_status_class(status) == beast::http::status_class::successful)
			return result;
		/* If we're being rate limited, wait and try again */
		if (status == beast::http::status::too_many_requests) {
			std::this_thread::sleep_for(std::chrono::milliseconds((uint64_t)(result.at("retry_after").as_double() * 1000.0)));
			return discord_http_request(method, path, auth, content);
		}
		/* If all else fails, throw */
		throw xDiscordError(result);
	}
	catch (const boost::system::system_error& e) {
		/* An HTTP or JSON parsing error occurred */
		throw xSystemError(e.what(), e.code().value());
	}
}

json::value cNet::HttpGet(const std::string &path, const std::string &auth) {
	return discord_http_request(beast::http::verb::get, path, auth, nullptr);
}
