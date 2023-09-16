#include "EmbedAuthor.h"
#include "json.h"

cEmbedAuthor::cEmbedAuthor(const json::value& v):
	m_name(json::value_to<std::string>(v.at("name"))) {
	const json::object& o = v.as_object();
	if (auto p = o.if_contains("url"))
		m_url = json::value_to<std::string>(*p);
	if (auto p = o.if_contains("icon_url"))
		m_icon_url = json::value_to<std::string>(*p);
	if (auto p = o.if_contains("proxy_icon_url"))
		m_proxy_icon_url = json::value_to<std::string>(*p);
}

void
tag_invoke(const json::value_from_tag&, json::value& v, const cEmbedAuthor& e) {
	v = {
		{ "name",     e.GetName()    },
		{ "url",      e.GetUrl()     },
		{ "icon_url", e.GetIconUrl() }
	};
}