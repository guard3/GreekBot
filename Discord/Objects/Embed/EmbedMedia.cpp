#include "EmbedMedia.h"
#include "json.h"

cEmbedMedia::cEmbedMedia(const json::object &o) {
	const json::value* v;
	if ((v = o.if_contains("url"      ))) m_url         = v->as_string().c_str();
	if ((v = o.if_contains("proxy_url"))) m_proxy_url   = v->as_string().c_str();
	if ((v = o.if_contains("width"    ))) m_width  = (int)v->as_int64();
	if ((v = o.if_contains("height"   ))) m_height = (int)v->as_int64();
}

cEmbedMedia::cEmbedMedia(const json::value& v) : cEmbedMedia(v.as_object()) {}

cEmbedMedia& cEmbedMedia::SetUrl(std::string url) noexcept {
	/* Set new url */
	m_url = std::move(url);
	/* Clear all other fields */
	m_proxy_url.clear();
	m_height = m_width = -1;
	return *this;
}

json::object
cEmbedMedia::ToJson() const { return {{ "url", m_url }}; }
