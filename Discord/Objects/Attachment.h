#ifndef GREEKBOT_ATTACHMENT_H
#define GREEKBOT_ATTACHMENT_H
#include "Base.h"

class cAttachment final {
private:
	cSnowflake m_id;
	std::string m_content_type;
	std::size_t m_size;
	std::string m_url;
	std::string m_proxy_url;
	int m_height;
	int m_width;
	// ...

public:
	explicit cAttachment(const boost::json::value&);
	explicit cAttachment(const boost::json::object&);

	const cSnowflake& GetId() const noexcept { return m_id; }
	std::string_view GetContentType() const noexcept { return m_content_type; }
	std::string_view GetUrl() const noexcept { return m_url; }
	std::string_view GetProxyUrl() const noexcept { return m_proxy_url; }
	std::size_t GetSize() const noexcept { return m_size; }
	int GetHeight() const noexcept { return m_height; }
	int GetWidth() const noexcept { return m_width; }

	std::string MoveUrl()      noexcept { return std::move(m_url);       }
	std::string MoveProxyUrl() noexcept { return std::move(m_proxy_url); }
};

cAttachment tag_invoke(boost::json::value_to_tag<cAttachment>, const boost::json::value&);
#endif /* GREEKBOT_ATTACHMENT_H */
