#ifndef GREEKBOT_EMBEDFOOTER_H
#define GREEKBOT_EMBEDFOOTER_H
#include "Common.h"

class cEmbedFooter final {
private:
	std::string m_text, m_icon_url, m_proxy_icon_url;

public:
	explicit cEmbedFooter(const json::value&);
	template<typename Str1 = std::string, typename Str2 = std::string>
	requires std::constructible_from<std::string, Str1&&> && std::constructible_from<std::string, Str2&&>
	cEmbedFooter(Str1&& text, Str2&& icon_url = {}): m_text(std::forward<Str1>(text)), m_icon_url(std::forward<Str2>(icon_url)) {}
	/* Getters */
	std::string_view GetText()         const noexcept { return m_text;           }
	std::string_view GetIconUrl()      const noexcept { return m_icon_url;       }
	std::string_view GetProxyIconUrl() const noexcept { return m_proxy_icon_url; }
	/* Movers */
	std::string MoveText()         noexcept { return std::move(m_text);           }
	std::string MoveIconUrl()      noexcept { return std::move(m_icon_url);       }
	std::string MoveProxyIconUrl() noexcept { return std::move(m_proxy_icon_url); }
	/* Setters */
	template<typename Str = std::string> requires std::assignable_from<std::string&, Str&&>
	cEmbedFooter& SetText(Str&& arg) {
		m_text = std::forward<Str>(arg);
		return *this;
	}
	template<typename Str = std::string> requires std::assignable_from<std::string&, Str&&>
	cEmbedFooter& SetIconUrl(Str&& arg) {
		m_icon_url = std::forward<Str>(arg);
		m_proxy_icon_url.clear();
		return *this;
	}
	/* Deleters */
	cEmbedFooter& ClearIconUrl() noexcept {
		m_icon_url.clear();
		m_proxy_icon_url.clear();
		return *this;
	}
};
typedef   hHandle<cEmbedFooter>   hEmbedFooter;
typedef  chHandle<cEmbedFooter>  chEmbedFooter;
typedef  uhHandle<cEmbedFooter>  uhEmbedFooter;
typedef uchHandle<cEmbedFooter> uchEmbedFooter;

void tag_invoke(const json::value_from_tag&, json::value&, const cEmbedFooter&);
#endif //GREEKBOT_EMBEDFOOTER_H