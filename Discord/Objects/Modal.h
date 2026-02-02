#ifndef GREEKBOT_MODAL_H
#define GREEKBOT_MODAL_H
#include "Component.h"

class cModal final {
private:
	std::string             m_custom_id;
	std::string             m_title;
	std::vector<cLayoutComponent> m_components;

public:
	cModal(std::string custom_id, std::string title, std::vector<cLayoutComponent> components):
		m_custom_id(std::move(custom_id)),
		m_title(std::move(title)),
		m_components(std::move(components)) {}

	friend void tag_invoke(boost::json::value_from_tag, boost::json::value&, const cModal&);
};

void tag_invoke(boost::json::value_from_tag, boost::json::value&, const cModal&);
#endif /* GREEKBOT_MODAL_H */