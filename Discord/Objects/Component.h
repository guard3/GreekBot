#ifndef DISCORD_COMPONENT_H
#define DISCORD_COMPONENT_H
#include "Component/Button.h"
#include "Component/SelectMenu.h"
#include "Component/TextInput.h"
#include "Component/UnsupportedComponent.h"
#include <span>
#include <variant>
/* The type of component; Exposed publicly because it's used in component interactions */
enum eComponentType {
	COMPONENT_ACTION_ROW = 1, // A container for other components
	COMPONENT_BUTTON,         // A button object
	COMPONENT_SELECT_MENU,    // A select menu for picking from choices
	COMPONENT_TEXT_INPUT      // A Text input object
};
eComponentType
tag_invoke(boost::json::value_to_tag<eComponentType>, const boost::json::value& v);
void
tag_invoke(boost::json::value_from_tag, boost::json::value&, eComponentType);
/* Define component as a variant of all types that may appear in an action row */
using cComponent = std::variant<cButton, cSelectMenu, cTextInput, cUnsupportedComponent>;
using   hComponent =   hHandle<cComponent>;
using  chComponent =  chHandle<cComponent>;
using  uhComponent =  uhHandle<cComponent>;
using uchComponent = uchHandle<cComponent>;
/* Provide custom JSON conversions for cComponent */
template<>
struct boost::json::is_variant_like<cComponent> : std::false_type {};
cComponent
tag_invoke(boost::json::value_to_tag<cComponent>, const boost::json::value&);
void
tag_invoke(boost::json::value_from_tag, boost::json::value&, const cComponent&);

class cActionRow final {
private:
	std::vector<cComponent> m_components;

public:
	explicit cActionRow(const boost::json::value&);
	explicit cActionRow(const boost::json::object&);

	template<std::convertible_to<cButton>... Buttons> requires (sizeof...(Buttons) < 6)
	explicit cActionRow(Buttons&&... buttons): m_components{ std::forward<Buttons>(buttons)... } {}
	template<std::convertible_to<cComponent> Comp>
	explicit cActionRow(Comp&& comp): m_components{ std::forward<Comp>(comp) } {}

	std::span<      cComponent> GetComponents()       noexcept { return m_components; }
	std::span<const cComponent> GetComponents() const noexcept { return m_components; }
};
typedef   hHandle<cActionRow>   hActionRow;
typedef  chHandle<cActionRow>  chActionRow;
typedef  uhHandle<cActionRow>  uhActionRow;
typedef uchHandle<cActionRow> uchActionRow;
cActionRow
tag_invoke(boost::json::value_to_tag<cActionRow>, const boost::json::value&);
void
tag_invoke(boost::json::value_from_tag, boost::json::value&, const cActionRow&);
#endif /* DISCORD_COMPONENT_H */