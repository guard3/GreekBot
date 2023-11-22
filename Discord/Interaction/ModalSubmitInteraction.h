#ifndef GREEKBOT_MODALSUBMITINTERACTION_H
#define GREEKBOT_MODALSUBMITINTERACTION_H
#include "Component.h"
#include "InteractionBase.h"
#include <span>
#include <vector>

class cModalSubmitInteraction final : public cInteraction {
private:
	std::string m_custom_id;
	std::vector<cActionRow> m_components;

	explicit cModalSubmitInteraction(const json::object&, const json::object&);
	using cInteraction::Visit;

public:
	explicit cModalSubmitInteraction(const json::value&);
	explicit cModalSubmitInteraction(const json::object&);

	std::string_view              GetCustomId() const noexcept { return m_custom_id;  }
	std::span<const cActionRow> GetComponents() const noexcept { return m_components; }

	std::span<cActionRow> GetComponents() noexcept { return m_components; }

	decltype(auto)   MoveCustomId() noexcept { return std::move(m_custom_id);  }
	decltype(auto) MoveComponents() noexcept { return std::move(m_components); }
};
typedef   hHandle<cModalSubmitInteraction>   hModalSubmitInteraction;
typedef  chHandle<cModalSubmitInteraction>  chModalSubmitInteraction;
typedef  uhHandle<cModalSubmitInteraction>  uhModalSubmitInteraction;
typedef uchHandle<cModalSubmitInteraction> uchModalSubmitInteraction;
#endif /* GREEKBOT_MODALSUBMITINTERACTION_H */
