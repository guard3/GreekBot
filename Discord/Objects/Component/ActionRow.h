#ifndef DISCORD_ACTIONROW_H
#define DISCORD_ACTIONROW_H
#include "ComponentBase.h"
#include "ComponentFwd.h"
#include <variant>
#include <vector>
#include <span>

/**
 * An Action Row is a top-level layout component.
 *
 * Action Rows can contain one of the following:
 * - Up to 5 contextually grouped buttons
 * - A single select component
 */
struct cActionRow : cComponentBase {
	using child_component_type = std::variant<cButton, cSelectMenu, cUnsupportedComponent>;

	explicit cActionRow(const boost::json::value&);
	explicit cActionRow(const boost::json::object&);

	cActionRow(std::initializer_list<child_component_type> components);

	template<iExplicitlyConvertibleTo<std::vector<child_component_type>> Components = std::vector<child_component_type>>
	explicit cActionRow(Components&& components) : m_components(std::forward<Components>(components)) {}

	/* Getters */
	auto GetComponents(this auto&& self) noexcept { return std::span(std::forward<decltype(self)>(self).m_components); }

	/* Movers */
	auto MoveComponents() noexcept { return std::move(m_components); }

private:
	std::vector<child_component_type> m_components;
};

/**
 * Disable the default JSON conversions for std::variant
 */
template<>
struct boost::json::is_variant_like<cActionRow::child_component_type> : std::false_type {};

/** @name JSON value to object conversion
 */
/// @{
cActionRow
tag_invoke(boost::json::value_to_tag<cActionRow>, const boost::json::value&);

cActionRow::child_component_type
tag_invoke(boost::json::value_to_tag<cActionRow::child_component_type>, const boost::json::value&);
/// @}

/** @name JSON value from object conversion
 */
/// @{
void
tag_invoke(boost::json::value_from_tag, boost::json::value&, const cActionRow&);

void
tag_invoke(boost::json::value_from_tag, boost::json::value&, const cActionRow::child_component_type&);
/// @}
#endif //DISCORD_ACTIONROW_H