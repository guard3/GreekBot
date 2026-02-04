#ifndef DISCORD_COMPONENT_H
#define DISCORD_COMPONENT_H
#include "Component/ActionRow.h"
#include "Component/Button.h"
#include "Component/SelectMenu.h"
#include "Component/TextInput.h"
#include "Component/UnsupportedComponent.h"
#include "Component/TextDisplay.h"
#include "Component/ComponentType.h"
#include "Component/Label.h"
#include "ComponentFwd.h"
#include <variant>

using cContentComponent = std::variant<cTextDisplay, cUnsupportedComponent>;

template<>
struct boost::json::is_variant_like<cContentComponent> : std::false_type {};

cContentComponent
tag_invoke(boost::json::value_to_tag<cContentComponent>, const boost::json::value&);

void
tag_invoke(boost::json::value_from_tag, boost::json::value&, const cContentComponent&);

///

using cLayoutPartialComponent = std::variant<cActionRow, cPartialLabel, cUnsupportedComponent>;

cLayoutPartialComponent
tag_invoke(boost::json::value_to_tag<cLayoutPartialComponent>, const boost::json::value&);

template<>
struct boost::json::is_variant_like<cLayoutPartialComponent> : std::false_type {};

using cLayoutComponent = std::variant<cActionRow, cLabel, cUnsupportedComponent>;

template<>
struct boost::json::is_variant_like<cLayoutComponent> : std::false_type {};

void
tag_invoke(boost::json::value_from_tag, boost::json::value&, const cLayoutComponent&);

#endif /* DISCORD_COMPONENT_H */