#ifndef DISCORD_LABEL_H
#define DISCORD_LABEL_H
#include "ComponentBase.h"
#include "ComponentFwd.h"
#include "TextInput.h"
#include "UnsupportedComponent.h"

using cLabelChildComponent = std::variant<cTextInput, cUnsupportedComponent>;

/**
 * Disable the default JSON conversions for std::variant
 */
template<>
struct boost::json::is_variant_like<cLabelChildComponent> : std::false_type {};

struct cPartialLabel : cComponentBase {
	using child_component_type = cLabelChildComponent;

	explicit cPartialLabel(const boost::json::value&);
	explicit cPartialLabel(const boost::json::object&);

	template<typename Self>
	auto&& GetComponent(this Self&& self) noexcept {
		return std::forward<Self>(self).m_component;
	}

protected:
	child_component_type m_component;

	template<typename Comp>
	explicit cPartialLabel(Comp&& comp) noexcept : m_component(std::forward<Comp>(comp)) {}
};

struct cLabel final : cPartialLabel {
	template<iExplicitlyConvertibleTo<std::string> Str1 = std::string,
	         iExplicitlyConvertibleTo<std::string> Str2 = std::string,
	         typename Comp>
	cLabel(Str1&& label, Comp&& comp, Str2&& description = {}) : cPartialLabel(std::forward<Comp>(comp)), m_label(std::forward<Str1>(label)), m_description(std::forward<Str2>(description)) {}

	std::string_view GetLabel() const noexcept { return m_label; }
	std::string_view GetDescription() const noexcept { return m_description; }

private:
	std::string m_label;
	std::string m_description;

	using cPartialLabel::cPartialLabel;
};

/** @name JSON value to object conversion
 */
/// @{
cLabelChildComponent
tag_invoke(boost::json::value_to_tag<cLabelChildComponent>, const boost::json::value&);

cPartialLabel
tag_invoke(boost::json::value_to_tag<cPartialLabel>, const boost::json::value&);
/// @}

/** @name JSON value from object conversion
 */
/// @{
void
tag_invoke(boost::json::value_from_tag, boost::json::value&, const cLabelChildComponent&);

void
tag_invoke(boost::json::value_from_tag, boost::json::value&, const cLabel&);
/// @}
#endif //DISCORD_LABEL_H