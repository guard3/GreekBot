#include "Modal.h"
#include "json.h"

json::object
cModal::ToJson() const {
	json::array a;
	a.reserve(m_components.size());
	for (auto& c : m_components)
		a.emplace_back(c.ToJson());

	return {
		{ "custom_id", m_custom_id },
		{ "title", m_title },
		{ "components", std::move(a)}
	};
}