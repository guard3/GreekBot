#include "Channel.h"
#include <boost/json.hpp>
/* ================================================================================================================== */
namespace json = boost::json;
/* ================================================================================================================== */
eChannelType
tag_invoke(json::value_to_tag<eChannelType>, const json::value& v) {
	return (eChannelType)v.to_number<std::underlying_type_t<eChannelType>>();
}
/* ================================================================================================================== */
cChannel::cChannel(const json::value& v) : cChannel(v.as_object()) {}
cChannel::cChannel(const json::object& o):
	id(o.at("id").as_string()),
	type(json::value_to<eChannelType>(o.at("type"))) {}