#ifndef DISCORD_MODALSUBMITINTERACTION_H
#define DISCORD_MODALSUBMITINTERACTION_H
#include "Component.h"
#include "InteractionBase.h"
#include <span>
#include <vector>

class cModalSubmitInteraction final : public cInteraction {
	using variant_type = std::variant<cActionRow, cPartialLabel, cUnsupportedComponent>;

public:
	struct component_type : cVariantComponentBase, variant_type {
		using variant_type::variant_type;
	};

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

template<>
struct boost::json::is_variant_like<cModalSubmitInteraction::component_type> : std::false_type {};

cModalSubmitInteraction::component_type
tag_invoke(boost::json::value_to_tag<cModalSubmitInteraction::component_type>, const boost::json::value&);
#endif /* DISCORD_MODALSUBMITINTERACTION_H */