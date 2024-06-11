#include "Attachment.h"
#include <boost/json.hpp>
/* ================================================================================================================== */
namespace json = boost::json;
/* ================================================================================================================== */
cAttachment::cAttachment(const json::value& v): cAttachment(v.as_object()) {}
cAttachment::cAttachment(const json::object& o):
	m_id(o.at("id").as_string()),
	m_content_type([&o] {
		auto p = o.if_contains("content_type");
		return p ? json::value_to<std::string>(*p) : std::string();
	}()),
	m_size(o.at("size").to_number<std::size_t>()),
	m_url(json::value_to<std::string>(o.at("url"))),
	m_proxy_url(json::value_to<std::string>(o.at("proxy_url"))),
	m_height([&o] {
		auto p = o.if_contains("height");
		return p && !p->is_null() ? p->to_number<int>() : -1;
	}()),
	m_width([&o] {
		auto p = o.if_contains("width");
		return p && !p->is_null() ? p->to_number<int>() : -1;
	}()) {}
/* ================================================================================================================== */
cAttachment
tag_invoke(json::value_to_tag<cAttachment>, const json::value& v) {
	return cAttachment{ v };
}