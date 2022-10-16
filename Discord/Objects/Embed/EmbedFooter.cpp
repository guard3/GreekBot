#include "EmbedFooter.h"
#include "json.h"

cEmbedFooter::cEmbedFooter(const json::object& o) : m_text(o.at("text").as_string().c_str()) {
	if (auto v = o.if_contains("icon_url"      )) m_icon_url       = v->as_string().c_str();
	if (auto v = o.if_contains("proxy_icon_url")) m_proxy_icon_url = v->as_string().c_str();
}

cEmbedFooter::cEmbedFooter(const json::value &v) : cEmbedFooter(v.as_object()) {}

json::object
cEmbedFooter::ToJson() const {
	return {
		{ "text",     m_text     },
		{ "icon_url", m_icon_url }
	};
}