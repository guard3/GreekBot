#include "ComponentFwd.h"
#include <boost/json.hpp>
#include <boost/throw_exception.hpp>

namespace sys = boost::system;
namespace json = boost::json;

eComponentType
tag_invoke(json::value_to_tag<eComponentType>, const json::value& v) {
	return static_cast<eComponentType>(v.to_number<std::underlying_type_t<eComponentType>>());
}

void
tag_invoke(json::value_from_tag, json::value& v, eComponentType t) {
	v = std::to_underlying(t);
}

void detail::throw_variant_exception() {
	boost::throw_exception(sys::system_error(json::error::exhausted_variants), BOOST_CURRENT_LOCATION);
}
