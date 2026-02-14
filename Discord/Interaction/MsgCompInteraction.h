#ifndef DISCORD_MSGCOMPINTERACTION_H
#define DISCORD_MSGCOMPINTERACTION_H
#include "Component.h"
#include "Component/ComponentType.h"
#include "InteractionBase.h"
#include "Message.h"
#include <span>
#include <vector>

class cMsgCompInteraction final : public cInteraction {
	cMessage m_message;
	std::string m_custom_id;
	eComponentType m_component_type;
	std::vector<std::string> m_values;

	explicit cMsgCompInteraction(const boost::json::object&, const boost::json::object&);
	using cInteraction::Visit;

public:
	explicit cMsgCompInteraction(const boost::json::value&);
	explicit cMsgCompInteraction(const boost::json::object&);

	/* Getters */
	std::string_view    GetCustomId() const noexcept { return m_custom_id;      }
	eComponentType GetComponentType() const noexcept { return m_component_type; }

	auto&& GetMessage(this auto&& self) noexcept { return std::forward<decltype(self)>(self).m_message; }
	auto    GetValues(this auto&& self) noexcept { return std::span(self.m_values);                     }

	/* Movers */
	auto  MoveMessage() noexcept { return std::move(m_message);   }
	auto MoveCustomId() noexcept { return std::move(m_custom_id); }
	auto   MoveValues() noexcept { return std::move(m_values);    }
};
#endif /* DISCORD_MSGCOMPINTERACTION_H */