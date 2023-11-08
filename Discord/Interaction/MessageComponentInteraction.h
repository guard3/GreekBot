#ifndef GREEKBOT_USERCOMMANDINTERACTION_H
#define GREEKBOT_USERCOMMANDINTERACTION_H
#include "InteractionBase.h"

class cMessageComponentInteraction final : public cInteraction {
private:
	cMessage m_message;
	std::string m_custom_id;
	eComponentType m_component_type;
	std::vector<std::string> m_values;

public:
	explicit cMessageComponentInteraction(const json::object&, const json::object&);

	const cMessage&             GetMessage() const noexcept { return m_message;        }
	std::string_view           GetCustomId() const noexcept { return m_custom_id;      }
	eComponentType        GetComponentType() const noexcept { return m_component_type; }
	std::span<const std::string> GetValues() const noexcept { return m_values;         }

	cMessage&             GetMessage() noexcept { return m_message; }
	std::span<std::string> GetValues() noexcept { return m_values;  }

	cMessage                MoveMessage() noexcept { return std::move(m_message);   }
	std::string            MoveCustomId() noexcept { return std::move(m_custom_id); }
	std::vector<std::string> MoveValues() noexcept { return std::move(m_values);    }
};
#endif //GREEKBOT_USERCOMMANDINTERACTION_H
