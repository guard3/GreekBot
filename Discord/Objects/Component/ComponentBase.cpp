#include "ComponentBase.h"
#include <boost/json.hpp>

namespace json = boost::json;

cComponentBase::cComponentBase(const json::value& v) : cComponentBase(v.as_object()) {}
cComponentBase::cComponentBase(const json::object& o) {
	if (const json::value* p = o.if_contains("id"))
		SetId(p->to_number<std::int32_t>());
}
