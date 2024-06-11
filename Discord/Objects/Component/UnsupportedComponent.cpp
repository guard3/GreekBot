#include "UnsupportedComponent.h"
#include <boost/json.hpp>
/* ================================================================================================================== */
namespace json = boost::json;
/* ========== Constructor definitions =============================================================================== */
cUnsupportedComponent::cUnsupportedComponent(const json::value& v): m_value(std::make_unique<json::value>(v)) {}
cUnsupportedComponent::cUnsupportedComponent(const cUnsupportedComponent& o): m_value(std::make_unique<json::value>(*o.m_value)) {}
cUnsupportedComponent::~cUnsupportedComponent() = default;
/* ========== Copy-assignment definition ============================================================================ */
cUnsupportedComponent&
cUnsupportedComponent::operator=(const cUnsupportedComponent& o) {
	*m_value = *o.m_value;
	return *this;
}
/* ========== JSON conversion overload definitions ================================================================== */
cUnsupportedComponent
tag_invoke(json::value_to_tag<cUnsupportedComponent>, const json::value& v) {
	return cUnsupportedComponent{ v };
}
void
tag_invoke(json::value_from_tag, json::value& v, const cUnsupportedComponent& c) {
	v = *c.m_value;
}