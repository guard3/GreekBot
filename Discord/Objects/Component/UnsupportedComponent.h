#ifndef DISCORD_UNSUPPORTEDCOMPONENT_H
#define DISCORD_UNSUPPORTEDCOMPONENT_H
#include "Common.h"
/* ========== A dummy component not directly supported ============================================================== */
class cUnsupportedComponent final {
	std::unique_ptr<json::value> m_value;

	friend cUnsupportedComponent
	tag_invoke(json::value_to_tag<cUnsupportedComponent>, const json::value&);
	friend void
	tag_invoke(json::value_from_tag, json::value&, const cUnsupportedComponent&);
public:
	explicit cUnsupportedComponent(const json::value&);
	cUnsupportedComponent(const cUnsupportedComponent&);
	cUnsupportedComponent(cUnsupportedComponent&&) noexcept = default;
	~cUnsupportedComponent();

	cUnsupportedComponent& operator=(const cUnsupportedComponent&);
	cUnsupportedComponent& operator=(cUnsupportedComponent&&) = default;
};
using   hUnsupportedComponent =   hHandle<cUnsupportedComponent>;
using  chUnsupportedComponent =  chHandle<cUnsupportedComponent>;
using  uhUnsupportedComponent =  uhHandle<cUnsupportedComponent>;
using uchUnsupportedComponent = uchHandle<cUnsupportedComponent>;
/* ========== JSON conversion overloads ============================================================================= */
cUnsupportedComponent
tag_invoke(json::value_to_tag<cUnsupportedComponent>, const json::value&);
void
tag_invoke(json::value_from_tag, json::value&, const cUnsupportedComponent&);
#endif /* DISCORD_UNSUPPORTEDCOMPONENT_H */
