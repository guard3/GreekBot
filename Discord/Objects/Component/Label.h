#ifndef DISCORD_LABEL_H
#define DISCORD_LABEL_H
#include "TextInput.h"
#include "UnsupportedComponent.h"

/**
 * Partial label is an interaction response object. It is received from modal interactions
 */
struct cPartialLabel : cComponentBase {
	static constexpr auto Type = COMPONENT_LABEL;
	using child_component_type = cVariantComponent<cPartialTextInput, cUnsupportedComponent>;

	explicit cPartialLabel(const boost::json::value&);
	explicit cPartialLabel(const boost::json::object&);

	auto&& GetComponent(this auto&& self) noexcept { return std::forward<decltype(self)>(self).m_component; }

private:
	child_component_type m_component;
};

/**
 * Label is a top-level layout component. Labels wrap modal components with text as a label and optional description.
 */
struct cLabel : cComponentBase {
	static constexpr auto Type = COMPONENT_LABEL;
	using child_component_type = cVariantComponent<cTextInput, cUnsupportedComponent>;

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

/** @name JSON value to object conversion
 */
/// @{
cPartialLabel
tag_invoke(boost::json::value_to_tag<cPartialLabel>, const boost::json::value&);
/// @}

/** @name JSON value from object conversion
 */
/// @{
void
tag_invoke(boost::json::value_from_tag, boost::json::value&, const cLabel&);
/// @}
#endif //DISCORD_LABEL_H
