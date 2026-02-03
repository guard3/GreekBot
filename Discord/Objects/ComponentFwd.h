#ifndef DISCORD_COMPONENTFWD_H
#define DISCORD_COMPONENTFWD_H
#include "BaseFwd.h"
#include <variant>

DISCORD_FWDDECL_CLASS(Button);
DISCORD_FWDDECL_CLASS(SelectMenu);
DISCORD_FWDDECL_CLASS(TextInput);
DISCORD_FWDDECL_CLASS(UnsupportedComponent);

/* Define component as a variant of all types that may appear in an action row */
using cComponent = std::variant<cButton, cSelectMenu, cUnsupportedComponent>;
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

#endif //DISCORD_COMPONENTFWD_H