#include "EmbedMedia.h"
#include "json.h"

cEmbedMedia::cEmbedMedia(const json::value& v) {
	const json::object& o = v.as_object();
	if (auto p = o.if_contains("url"))
		m_url = json::value_to<std::string>(*p);
	if (auto p = o.if_contains("proxy_url"))
		m_proxy_url = json::value_to<std::string>(*p);
	if (auto p = o.if_contains("width"))
		m_width = p->to_number<int>();
	if (auto p = o.if_contains("height"))
		m_height = p->to_number<int>();
}

cEmbedMedia&
cEmbedMedia::SetUrl(std::string url) noexcept {
	/* Set new url */
	m_url = std::move(url);
	/* Clear all other fields */
	m_proxy_url.clear();
	m_height = m_width = -1;
	return *this;
}

void
tag_invoke(const json::value_from_tag&, json::value& v, const cEmbedMedia& e) {
	v = {{ "url", e.GetUrl() }};
}
