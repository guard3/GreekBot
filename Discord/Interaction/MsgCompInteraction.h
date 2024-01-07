#ifndef DISCORD_MSGCOMPINTERACTION_H
#define DISCORD_MSGCOMPINTERACTION_H
#include "Component.h"
#include "InteractionBase.h"
#include "Message.h"
#include <span>
#include <vector>

class cMsgCompInteraction final : public cInteraction {
private:
	cMessage m_message;
	std::string m_custom_id;
	eComponentType m_component_type;
	std::vector<std::string> m_values;

	explicit cMsgCompInteraction(const json::object&, const json::object&);
	using cInteraction::Visit;

public:
	explicit cMsgCompInteraction(const json::value&);
	explicit cMsgCompInteraction(const json::object&);

	const cMessage&             GetMessage() const noexcept { return m_message;        }
	std::string_view           GetCustomId() const noexcept { return m_custom_id;      }
	eComponentType        GetComponentType() const noexcept { return m_component_type; }
	std::span<const std::string> GetValues() const noexcept { return m_values;         }

	cMessage&             GetMessage() noexcept { return m_message; }
	std::span<std::string> GetValues() noexcept { return m_values;  }

	decltype(auto)  MoveMessage() noexcept { return std::move(m_message);   }
	decltype(auto) MoveCustomId() noexcept { return std::move(m_custom_id); }
	decltype(auto)   MoveValues() noexcept { return std::move(m_values);    }
};
#endif /* DISCORD_MSGCOMPINTERACTION_H */