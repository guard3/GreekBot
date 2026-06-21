#ifndef DISCORD_UNSUPPORTEDCOMPONENT_H
#define DISCORD_UNSUPPORTEDCOMPONENT_H
#include "ComponentBase.h"

/* ========== A dummy component not directly supported ============================================================== */
class cUnsupportedComponent final {
	std::unique_ptr<boost::json::value> m_value;

	friend cUnsupportedComponent
	tag_invoke(boost::json::value_to_tag<cUnsupportedComponent>, const boost::json::value&);
	friend void
	tag_invoke(boost::json::value_from_tag, boost::json::value&, const cUnsupportedComponent&);
public:
	static constexpr auto Type = COMPONENT_UNSUPPORTED;

	explicit cUnsupportedComponent(const boost::json::value&);
	cUnsupportedComponent(const cUnsupportedComponent&);
	cUnsupportedComponent(cUnsupportedComponent&&) noexcept;
	~cUnsupportedComponent();

	cUnsupportedComponent& operator=(const cUnsupportedComponent&);
	cUnsupportedComponent& operator=(cUnsupportedComponent&&) noexcept;
};
/* ========== JSON conversion overloads ============================================================================= */
cUnsupportedComponent
tag_invoke(boost::json::value_to_tag<cUnsupportedComponent>, const boost::json::value&);
void
tag_invoke(boost::json::value_from_tag, boost::json::value&, const cUnsupportedComponent&);
#endif /* DISCORD_UNSUPPORTEDCOMPONENT_H */
