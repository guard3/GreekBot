#ifndef DISCORD_LABEL_H
#define DISCORD_LABEL_H
#include "ComponentBase.h"
#include "ComponentFwd.h"
#include "TextInput.h"
#include "UnsupportedComponent.h"
#include <variant>

/**
 * Partial label is an interaction response object. It is received from modal interactions
 */
struct cPartialLabel : cComponentBase {
	using child_component_type = std::variant<cPartialTextInput, cUnsupportedComponent>;

	explicit cPartialLabel(const boost::json::value&);
	explicit cPartialLabel(const boost::json::object&);

	auto&& GetComponent(this auto&& self) noexcept { return std::forward<decltype(self)>(self).m_component; }

private:
	child_component_type m_component;
};

/**
 * Label is a top-level layout component. Labels wrap modal components with text as a label and optional description.
 */
struct cLabel final : cComponentBase {
	using child_component_type = std::variant<cTextInput, cUnsupportedComponent>;

	template<iExplicitlyConvertibleTo<std::string> Str1 = std::string,
	         iExplicitlyConvertibleTo<std::string> Str2 = std::string,
	         typename Comp>
	cLabel(Str1&& label, Comp&& comp, Str2&& description = {}) : m_label(std::forward<Str1>(label)), m_description(std::forward<Str2>(description)), m_component(std::forward<Comp>(comp)) {}

	std::string_view GetLabel() const noexcept { return m_label; }
	std::string_view GetDescription() const noexcept { return m_description; }
	auto&& GetComponent(this auto&& self) noexcept { return std::forward<decltype(self)>(self).m_component; }

private:
	std::string m_label;
	std::string m_description;
	child_component_type m_component;
};

/**
 * Disable the default JSON conversions for std::variant
 */
template<>
struct boost::json::is_variant_like<cPartialLabel::child_component_type> : std::false_type {};

template<>
struct boost::json::is_variant_like<cLabel::child_component_type> : std::false_type {};

/** @name JSON value to object conversion
 */
/// @{
cPartialLabel
tag_invoke(boost::json::value_to_tag<cPartialLabel>, const boost::json::value&);

cPartialLabel::child_component_type
tag_invoke(boost::json::value_to_tag<cPartialLabel::child_component_type>, const boost::json::value&);
/// @}

/** @name JSON value from object conversion
 */
/// @{
void
tag_invoke(boost::json::value_from_tag, boost::json::value&, const cLabel&);

void
tag_invoke(boost::json::value_from_tag, boost::json::value&, const cLabel::child_component_type&);
/// @}
#endif //DISCORD_LABEL_H