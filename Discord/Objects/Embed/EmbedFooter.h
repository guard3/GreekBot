#ifndef GREEKBOT_EMBEDFOOTER_H
#define GREEKBOT_EMBEDFOOTER_H
#include "Common.h"

class cEmbedFooter final {
private:
	std::string m_text, m_icon_url, m_proxy_icon_url;

public:
	explicit cEmbedFooter(const json::value&);
	cEmbedFooter(const char* text) : m_text(text) {}
	cEmbedFooter(std::string text) : m_text(std::move(text)) {}
	cEmbedFooter(std::string text, std::string icon_url) : m_text(std::move(text)), m_icon_url(std::move(icon_url)) {}
	/* Getters */
	const std::string& GetText()         const noexcept { return m_text;           }
	const std::string& GetIconUrl()      const noexcept { return m_icon_url;       }
	const std::string& GetProxyIconUrl() const noexcept { return m_proxy_icon_url; }
	/* Movers */
	std::string MoveText()         noexcept { return std::move(m_text);           }
	std::string MoveIconUrl()      noexcept { return std::move(m_icon_url);       }
	std::string MoveProxyIconUrl() noexcept { return std::move(m_proxy_icon_url); }
	/* Setters */
	cEmbedFooter& SetText(std::string text) {
		m_text = std::move(text);
		return *this;
	}
	cEmbedFooter& SetIconUrl(std::string url) {
		m_icon_url = std::move(url);
		m_proxy_icon_url.clear();
		return *this;
	}
	/* Deleters */
	cEmbedFooter& ClearIconUrl() {
		m_proxy_icon_url.clear();
		m_icon_url = nullptr;
		return *this;
	}
};
typedef   hHandle<cEmbedFooter>   hEmbedFooter;
typedef  chHandle<cEmbedFooter>  chEmbedFooter;
typedef  uhHandle<cEmbedFooter>  uhEmbedFooter;
typedef uchHandle<cEmbedFooter> uchEmbedFooter;
typedef  shHandle<cEmbedFooter>  shEmbedFooter;
typedef schHandle<cEmbedFooter> schEmbedFooter;

void tag_invoke(const json::value_from_tag&, json::value&, const cEmbedFooter&);
#endif //GREEKBOT_EMBEDFOOTER_H