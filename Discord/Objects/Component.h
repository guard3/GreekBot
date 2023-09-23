#ifndef GREEKBOT_COMPONENT_H
#define GREEKBOT_COMPONENT_H
#include "Component/Button.h"
#include "Component/SelectMenu.h"
#include "Component/TextInput.h"
#include <span>
#include <variant>

enum eComponentType {
	COMPONENT_ACTION_ROW = 1, // A container for other components
	COMPONENT_BUTTON,         // A button object
	COMPONENT_SELECT_MENU,    // A select menu for picking from choices
	COMPONENT_TEXT_INPUT      // A Text input object
};

using cComponent = std::variant<cButton, cLinkButton, cSelectMenu, cTextInput>;

class cActionRow {
private:
	std::vector<cComponent> m_components;

public:
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
typedef  shHandle<cActionRow>  shActionRow;
typedef schHandle<cActionRow> schActionRow;

KW_DECLARE(components, std::vector<cActionRow>)

void tag_invoke(const json::value_from_tag&, json::value&, const cActionRow&);
#endif /* GREEKBOT_COMPONENT_H */