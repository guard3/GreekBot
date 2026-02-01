#include "ComponentType.h"
#include "TextDisplay.h"
#include <boost/json.hpp>

namespace json = boost::json;

cTextDisplay::cTextDisplay(const json::value& v) : cTextDisplay(v.as_object()) {}
cTextDisplay::cTextDisplay(const json::object& o): cComponentBase(o), m_content(json::value_to<std::string>(o.at("content"))) {}

cTextDisplay
tag_invoke(json::value_to_tag<cTextDisplay>, const json::value& v) {
	return cTextDisplay{ v };
}

void
tag_invoke(json::value_from_tag, json::value& v, const cTextDisplay& cmp) {
	v = {
		{ "type",    COMPONENT_TEXT_DISPLAY },
		{ "id",      cmp.GetId()            },
		{ "content", cmp.GetContent()       }
	};
}
