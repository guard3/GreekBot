#include "Attachment.h"
#include "json.h"

cAttachment::cAttachment(const json::value& v):
	m_id(json::value_to<std::string_view>(v.at("id"))),
	m_size(v.at("size").to_number<size_t>()),
	m_url(json::value_to<std::string>(v.at("url"))),
	m_proxy_url(json::value_to<std::string>(v.at("proxy_url"))),
	m_height(-1),
	m_width(-1) {
	auto& o = v.get_object();
	if (auto p = o.if_contains("content_type"))
		m_content_type = json::value_to<std::string>(*p);
	if (auto p = o.if_contains("height")) {
		auto res = json::try_value_to<int>(*p);
		if (res.has_value())
			m_height = res.value();
	}
	if (auto p = o.if_contains("width")) {
		auto res = json::try_value_to<int>(*p);
		if (res.has_value())
			m_width = res.value();
	}
}

cAttachment
tag_invoke(const json::value_to_tag<cAttachment>&, const json::value& v) {
	return cAttachment{ v };
}