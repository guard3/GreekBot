#include "Channel.h"
#include "json.h"

cChannel::cChannel(const json::object& o) : id(o.at("id").as_string().c_str()),	type((eChannelType)o.at("type").as_int64()) {}

cChannel::cChannel(const json::value& v) : cChannel(v.as_object()) {}
