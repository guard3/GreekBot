#include "EmbedFooter.h"
#include "json.h"

cEmbedFooter::cEmbedFooter(const json::value& v):
	m_text(json::value_to<std::string>(v.at("text"))) {
	const json::object& o = v.as_object();
	if (auto p = o.if_contains("icon_url"))
		m_icon_url = json::value_to<std::string>(*p);
	if (auto p = o.if_contains("proxy_icon_url"))
		m_proxy_icon_url = json::value_to<std::string>(*p);
}

void
tag_invoke(const json::value_from_tag&, json::value& v, const cEmbedFooter& e) {
	v = {
		{ "text",     e.GetText()    },
		{ "icon_url", e.GetIconUrl() }
	};
}