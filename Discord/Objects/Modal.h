#ifndef GREEKBOT_MODAL_H
#define GREEKBOT_MODAL_H
#include "Component.h"

class cModal final {
private:
	std::string             m_custom_id;
	std::string             m_title;
	std::vector<cActionRow> m_components;

public:
	cModal(std::string custom_id, std::string title, std::vector<cActionRow> components):
		m_custom_id(std::move(custom_id)),
		m_title(std::move(title)),
		m_components(std::move(components)) {}

	json::object ToJson() const;
};

#endif /* GREEKBOT_MODAL_H */