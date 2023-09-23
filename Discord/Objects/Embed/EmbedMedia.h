#ifndef GREEKBOT_EMBEDMEDIA_H
#define GREEKBOT_EMBEDMEDIA_H
#include "Common.h"

class cEmbedMedia final {
private:
	std::string m_url;         // The source url of the image or video
	std::string m_proxy_url;   // A proxied url of the image or video
	int         m_width  = -1; // Width
	int         m_height = -1; // Height

public:
	explicit cEmbedMedia(const json::value&);
	template<typename Str = std::string> requires std::constructible_from<std::string, Str&&>
	cEmbedMedia(Str&& url): m_url(std::forward<Str>(url)) {}
	/* Getters */
	std::string_view GetUrl()      const noexcept { return m_url;       }
	std::string_view GetProxyUrl() const noexcept { return m_proxy_url; }
	int              GetWidth()    const noexcept { return m_width;     }
	int              GetHeight()   const noexcept { return m_height;    }
	/* Movers */
	std::string MoveUrl()      noexcept { return std::move(m_url);       }
	std::string MoveProxyUrl() noexcept { return std::move(m_proxy_url); }
	/* Setters */
	template<typename Str = std::string> requires std::assignable_from<std::string&, Str&&>
	cEmbedMedia& SetUrl(Str&& url) {
		m_url = std::forward<Str>(url);
		m_proxy_url.clear();
		m_height = m_width = -1;
		return *this;
	}
};
typedef   hHandle<cEmbedMedia>   hEmbedMedia;
typedef  chHandle<cEmbedMedia>  chEmbedMedia;
typedef  uhHandle<cEmbedMedia>  uhEmbedMedia;
typedef uchHandle<cEmbedMedia> uchEmbedMedia;

void tag_invoke(const json::value_from_tag&, json::value&, const cEmbedMedia&);
#endif //GREEKBOT_EMBEDMEDIA_H