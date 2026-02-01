#include "ComponentType.h"
#include <boost/json.hpp>

namespace json = boost::json;

eComponentType
tag_invoke(json::value_to_tag<eComponentType>, const json::value& v) {
	return static_cast<eComponentType>(v.to_number<std::underlying_type_t<eComponentType>>());
}

void
tag_invoke(json::value_from_tag, json::value& v, eComponentType t) {
	v = std::to_underlying(t);
}