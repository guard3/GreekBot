#include "Interaction.h"

cMessageComponentInteractionData::cMessageComponentInteractionData(const json::value& v) : component_type(static_cast<eComponentType>(v.at("component_type").as_int64())), custom_id(nullptr) {
	if (auto o = v.if_object()) {
		/* Initialize custom_id */
		if (auto c = o->if_contains("custom_id")) {
			if (auto s = c->if_string()) {
				custom_id = new char[s->size() + 1];
				strcpy(custom_id, s->c_str());
			}
		}
		/* Initialize values */
		if (auto c = o->if_contains("values")) {
			if (auto a = c->if_array()) {
				auto& values = const_cast<std::vector<const char*>&>(Values);
				values.reserve(a->size());
				for (auto& e : *a) {
					if (auto s = e.if_string()) {
						char* tmp = new char[s->size() + 1];
						strcpy(tmp, s->c_str());
						values.push_back(tmp);
					}
					else {
						values.clear();
						break;
					}
				}
			}
		}
	}
}

cMessageComponentInteractionData::cMessageComponentInteractionData(const cMessageComponentInteractionData& o) : component_type(o.component_type), custom_id(nullptr) {
	/* Initialize custom_id */
	if (o.custom_id) {
		custom_id = new char[strlen(o.custom_id) + 1];
		strcpy(custom_id, o.custom_id);
	}
	/* Initialize values */
	if (!o.Values.empty()) {
		auto& values = const_cast<std::vector<const char*>&>(Values);
		values.reserve(o.Values.size());
		for (const char* s : o.Values) {
			char* tmp = new char[strlen(s) + 1];
			strcpy(tmp, s);
			values.push_back(tmp);
		}
	}
}

cMessageComponentInteractionData::cMessageComponentInteractionData(cMessageComponentInteractionData &&o) noexcept : component_type(o.component_type), Values(std::move(const_cast<std::vector<const char*>&>(o.Values))) {
	custom_id = o.custom_id;
	o.custom_id = nullptr;
}

cMessageComponentInteractionData::~cMessageComponentInteractionData() {
	delete[] custom_id;
	for (auto s : Values)
		delete[] s;
}

cMessageComponentInteractionData& cMessageComponentInteractionData::operator=(cMessageComponentInteractionData o) {
	auto s = custom_id;
	custom_id = o.custom_id;
	o.custom_id = s;
	component_type = o.component_type;
	const_cast<std::vector<const char*>&>(Values) = std::move(const_cast<std::vector<const char*>&>(o.Values));
	return *this;
}