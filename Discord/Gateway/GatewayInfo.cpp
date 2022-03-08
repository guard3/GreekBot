#include "GatewayInfo.h"
#include "json.h"

cSessionStartLimit::cSessionStartLimit(const json::object& o) : m_values {
	(int)o.at("total"          ).as_int64(),
	(int)o.at("remaining"      ).as_int64(),
	(int)o.at("reset_after"    ).as_int64(),
	(int)o.at("max_concurrency").as_int64()
} {}

cSessionStartLimit::cSessionStartLimit(const json::value& v) : cSessionStartLimit(v.as_object()) {}

cGatewayInfo::cGatewayInfo(const json::object &o) : url(o.at("url").as_string().c_str()), shards(o.at("shards").as_int64()), session_start_limit(o.at("session_start_limit")) {}

cGatewayInfo::cGatewayInfo(const json::value &v) : cGatewayInfo(v.as_object()) {}