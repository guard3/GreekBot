#include "EmbedAuthor.h"
#include "json.h"

cEmbedAuthor::cEmbedAuthor(const json::object& o) : m_name(o.at("name").as_string().c_str()) {
	if (auto v = o.if_contains("url"           )) m_url            = v->as_string().c_str();
	if (auto v = o.if_contains("icon_url"      )) m_icon_url       = v->as_string().c_str();
	if (auto v = o.if_contains("proxy_icon_url")) m_proxy_icon_url = v->as_string().c_str();
}

cEmbedAuthor::cEmbedAuthor(const json::value &v) : cEmbedAuthor(v.as_object()) {}

json::object
cEmbedAuthor::ToJson() const {
	return {
		{ "name",     m_name     },
		{ "url",      m_url      },
		{ "icon_url", m_icon_url }
	};
}