#ifndef GREEKBOT_MODAL_H
#define GREEKBOT_MODAL_H
#include "Component.h"

struct cModal {
	using component_type = cVariantComponent<cActionRow, cLabel, cUnsupportedComponent>;

	cModal(std::string custom_id, std::string title, std::vector<component_type> components):
		m_custom_id(std::move(custom_id)),
		m_title(std::move(title)),
		m_components(std::move(components)) {}

	std::string_view GetCustomId() const noexcept { return m_custom_id; }
	std::string_view GetTitle() const noexcept { return m_title; }
	auto GetComponents(this auto&& self) noexcept { return std::span(self.m_components); }

private:
	std::string                 m_custom_id;
	std::string                 m_title;
	std::vector<component_type> m_components;
};

void
tag_invoke(boost::json::value_from_tag, boost::json::value&, const cModal&);
#endif /* GREEKBOT_MODAL_H */