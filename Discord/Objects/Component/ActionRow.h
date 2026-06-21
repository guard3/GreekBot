#ifndef DISCORD_ACTIONROW_H
#define DISCORD_ACTIONROW_H
#include "Button.h"
#include "SelectMenu.h"
#include "UnsupportedComponent.h"
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
	static constexpr auto Type = COMPONENT_ACTION_ROW;

	using child_component_type = cVariantComponent<cButton, cSelectMenu, cUnsupportedComponent>;

	explicit cActionRow(const boost::json::value&);
	explicit cActionRow(const boost::json::object&);

	cActionRow(std::initializer_list<child_component_type> list) : m_components(list) {}

	template<iExplicitlyConvertibleTo<std::vector<child_component_type>> Components = std::vector<child_component_type>>
	explicit cActionRow(Components&& components) : m_components(std::forward<Components>(components)) {}

	/* Getters */
	auto GetComponents(this auto&& self) noexcept { return std::span(std::forward<decltype(self)>(self).m_components); }

	/* Movers */
	auto MoveComponents() noexcept { return std::move(m_components); }

private:
	std::vector<child_component_type> m_components;
};

/** @name JSON value to object conversion
 */
/// @{
cActionRow
tag_invoke(boost::json::value_to_tag<cActionRow>, const boost::json::value&);
/// @}

/** @name JSON value from object conversion
 */
/// @{
void
tag_invoke(boost::json::value_from_tag, boost::json::value&, const cActionRow&);
/// @}
#endif //DISCORD_ACTIONROW_H
