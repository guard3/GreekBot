#ifndef DISCORD_MODALSUBMITINTERACTION_H
#define DISCORD_MODALSUBMITINTERACTION_H
#include "Component.h"
#include "InteractionBase.h"
#include <span>
#include <vector>

struct cModalSubmitInteraction : cInteraction {
	using component_type = cVariantComponent<cActionRow, cPartialLabel, cUnsupportedComponent>;

	explicit cModalSubmitInteraction(const boost::json::value&);
	explicit cModalSubmitInteraction(const boost::json::object&);

	std::string_view GetCustomId() const noexcept { return m_custom_id;  }
	auto GetComponents(this auto&& self) noexcept { return std::span(self.m_components); }

	auto   MoveCustomId() noexcept { return std::move(m_custom_id);  }
	auto MoveComponents() noexcept { return std::move(m_components); }

private:
	std::string m_custom_id;
	std::vector<component_type> m_components;

	explicit cModalSubmitInteraction(const boost::json::object&, const boost::json::object&);
	using cInteraction::Visit;
};
#endif /* DISCORD_MODALSUBMITINTERACTION_H */
