#include "Permission.h"
#include "Utils.h"

#include <boost/json.hpp>

namespace json = boost::json;

ePermission
tag_invoke(json::value_to_tag<ePermission>, const json::value& v) {
	return ePermission(cUtils::ParseInt<std::uint64_t>(v.as_string()));
}