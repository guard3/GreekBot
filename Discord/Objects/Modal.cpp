#include "Modal.h"
#include "json.h"

void
tag_invoke(const json::value_from_tag&, json::value& v, const cModal& modal) {
	json::object& obj = v.emplace_object();
	obj = {
		{ "custom_id", modal.m_custom_id },
		{ "title",     modal.m_title     }
	};
	json::value_from(modal.m_components, obj["components"]);
}