#include "GatewayInfo.h"

cGatewayInfo::cGatewayInfo(const json::value& v) {
	try {
		url = v.at("url").as_string();
		shards = static_cast<int>(v.at("shards").as_int64());
		session_start_limit = new cSessionStartLimit(v.at("session_start_limit"));
	}
	catch (const std::exception&) {
		error = new cJsonError(v);
		url.clear();
		shards = 0;
	}
}

cGatewayInfo::cGatewayInfo(const cGatewayInfo& other) :
	error(other.error ? new cJsonError(*other.error) : nullptr),
	session_start_limit(other.session_start_limit ? new cSessionStartLimit(*other.session_start_limit) : nullptr),
	shards(other.shards),
	url(other.url) {}

cGatewayInfo::cGatewayInfo(cGatewayInfo&& other) : error(other.error), session_start_limit(other.session_start_limit), shards(other.shards), url(other.url) {
	other.error               = nullptr;
	other.session_start_limit = nullptr;
	other.shards              = 0;
	other.url.clear();
}

cGatewayInfo::~cGatewayInfo() {
	delete error;
	delete session_start_limit;
}

cGatewayInfo& cGatewayInfo::operator=(cGatewayInfo other) {
	auto v1 = other.error;
	auto v2 = other.session_start_limit;
	other.error = error;
	other.session_start_limit = session_start_limit;
	error = v1;
	session_start_limit = v2;
	shards = other.shards;
	url = other.url;
	return *this;
}
