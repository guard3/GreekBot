#include "Application.h"
#include <boost/json.hpp>
/* ================================================================================================================== */
namespace json = boost::json;
/* ================================================================================================================== */
eApplicationFlag
tag_invoke(json::value_to_tag<eApplicationFlag>, const json::value& v) {
	return static_cast<eApplicationFlag>(v.to_number<int>());
}
/* ================================================================================================================== */
cApplication::cApplication(const json::value& v): cApplication(v.as_object()) {}
cApplication::cApplication(const json::object& o):
	m_id(o.at("id").as_string()),
	m_flags(json::value_to<eApplicationFlag>(o.at("flags"))) {}
/* ================================================================================================================== */
cApplication
tag_invoke(json::value_to_tag<cApplication>, const json::value& v) {
	return cApplication{ v };
}