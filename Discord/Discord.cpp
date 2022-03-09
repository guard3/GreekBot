#include "Discord.h"
#include "beast_http.h"
#include "json.h"
#include <tuple>

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

static std::tuple<beast::http::status, json::value> http_request(beast::http::verb method, const std::string& path, const std::string& auth, const json::object* content) {
	/* The IO context for net operations */
	asio::io_context ioc;
	/* The SSL context */
	asio::ssl::context ctx(asio::ssl::context::tlsv13_client);
	/* TODO: Fix certificates and shit */
	//ctx.set_verify_mode(ssl::verify_peer);
	/* Create an SSL stream and connect to host */
	beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);
	beast::get_lowest_layer(stream).connect(asio::ip::tcp::resolver(ioc).resolve(DISCORD_API_HOST, "https"));
	/* Perform SSL handshake */
	stream.handshake(asio::ssl::stream_base::client);
	/* Prepare the HTTP request */
	beast::http::request<beast::http::string_body> request(method, DISCORD_API_ENDPOINT + path, 11);
	request.set(beast::http::field::host, DISCORD_API_HOST);
	request.set(beast::http::field::user_agent, "GreekBot");
	request.set(beast::http::field::authorization, auth);
	if (content) {
		std::string str = json::serialize(*content);
		request.set(beast::http::field::content_type, "application/json");
		request.set(beast::http::field::content_length, std::to_string(str.length()));
		request.body() = std::move(str);
	}
	else request.set(beast::http::field::content_length, "0");
	beast::http::write(stream, request);
	/* Read the request response */
	beast::flat_buffer buffer;
	beast::http::response<beast::http::string_body> res;
	beast::http::read(stream, buffer, res);
	/* Shut down the stream */
	beast::error_code e;
	stream.shutdown(e);
	auto f = res.find(beast::http::field::content_type);
	return { res.result(), f == res.end() ? json::value() : f->value() == "application/json" ? json::parse(res.body()) : json::object() };
}


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

json::value
cDiscord::HttpGet(const std::string &path, const std::string &auth) {
	return discord_http_request(beast::http::verb::get, path, auth, nullptr);
}

void
cDiscord::HttpPost(const std::string &path, const std::string &auth, const json::object &obj) {
	discord_http_request(beast::http::verb::post, path, auth, &obj);
}

json::value
cDiscord::HttpPatch(const std::string &path, const std::string &auth, const json::object &obj) {
	return discord_http_request(beast::http::verb::patch, path, auth, &obj);
}

json::value
cDiscord::HttpPut(const std::string &path, const std::string &auth) {
	return discord_http_request(beast::http::verb::put, path, auth, nullptr);
}

json::value
cDiscord::HttpPut(const std::string &path, const std::string &auth, const json::object& obj) {
	return discord_http_request(beast::http::verb::put, path, auth, &obj);
}

json::value
cDiscord::HttpDelete(const std::string &path, const std::string &auth) {
	return discord_http_request(beast::http::verb::delete_, path, auth, nullptr);
}