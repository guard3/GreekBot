#include "Interaction.h"

cMessageComponentInteractionData::cMessageComponentInteractionData(const json::value& v) : component_type(static_cast<eComponentType>(v.at("component_type").as_int64())) {
	uhHandle<char[]> u_custom_id;
	try {
		/* Initialize custom_id */
		auto& s = v.at("custom_id").as_string();
		u_custom_id = cHandle::MakeUnique<char[]>(s.size() + 1);
		strcpy(u_custom_id.get(), s.c_str());
		/* Initialize values */
		auto& a = v.at("values").as_array();
		values.reserve(a.size());
		Values.reserve(a.size());
		for (auto& e : a) {
			values.emplace_back(e.as_string().c_str());
			Values.push_back(values.back().c_str());
		}
	}
	catch (...) {}
	/* Copy pointers */
	custom_id = u_custom_id.release();
}

cMessageComponentInteractionData::cMessageComponentInteractionData(const cMessageComponentInteractionData& o) : component_type(o.component_type), values(o.values) {
	/* Initialize custom_id */
	uhHandle<char[]> u_custom_id;
	if (o.custom_id) {
		u_custom_id = cHandle::MakeUnique<char[]>(strlen(o.custom_id) + 1);
		strcpy(u_custom_id.get(), o.custom_id);
	}
	/* Initialize values */
	Values.reserve(values.size());
	for (auto& e : values)
		Values.push_back(e.c_str());
	/* Copy pointers */
	custom_id = u_custom_id.release();
}

cMessageComponentInteractionData::cMessageComponentInteractionData(cMessageComponentInteractionData &&o) noexcept : component_type(o.component_type), values(std::move(o.values)), Values(std::move(o.Values)) {
	custom_id = o.custom_id;
	o.custom_id = nullptr;
}

cMessageComponentInteractionData::~cMessageComponentInteractionData() {
	delete[] custom_id;
}

cMessageComponentInteractionData& cMessageComponentInteractionData::operator=(cMessageComponentInteractionData o) {
	std::swap(custom_id, o.custom_id);
	component_type = o.component_type;
	values.swap(values);
	Values.swap(Values);
	return *this;
}