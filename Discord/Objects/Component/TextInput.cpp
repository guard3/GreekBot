#include "TextInput.h"
#include "json.h"

cTextInput::cTextInput(const json::value& v) : cTextInput(v.as_object()) {}
cTextInput::cTextInput(const json::object& o) : //TODO: complete this...
	m_custom_id(json::value_to<std::string>(o.at("custom_id"))),
	m_value(json::value_to<std::string>(o.at("value"))) {}

cTextInput
tag_invoke(json::value_to_tag<cTextInput>, const json::value& v) {
	return cTextInput{ v };
}