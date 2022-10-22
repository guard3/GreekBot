#include "Interaction.h"
#include "json.h"

cInteractionData<INTERACTION_APPLICATION_COMMAND>::cInteractionData(const json::object& o) : id(o.at("id")), name(o.at("name").as_string().c_str()), type((eApplicationCommandType)o.at("type").as_int64()) {
	if (auto p = o.if_contains("options")) {
		auto& a = p->as_array();
		auto  r = o.if_contains("resolved");
		Options.reserve(a.size());
		for (auto &v: a)
			Options.emplace_back(v, r);
	}
}
cInteractionData<INTERACTION_APPLICATION_COMMAND>::cInteractionData(const json::value& v) : cInteractionData(v.as_object()) {}

cInteractionData<INTERACTION_MESSAGE_COMPONENT>::cInteractionData(const json::object& o) : custom_id(o.at("custom_id").as_string().c_str()), component_type((eComponentType)o.at("component_type").as_int64()) {
	if (auto p = o.if_contains("values"))
		Values = json::value_to<std::vector<std::string>>(*p);
}
cInteractionData<INTERACTION_MESSAGE_COMPONENT>::cInteractionData(const json::value& v) : cInteractionData(v.as_object()) {}