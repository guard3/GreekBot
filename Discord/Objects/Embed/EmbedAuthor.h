#ifndef GREEKBOT_EMBEDAUTHOR_H
#define GREEKBOT_EMBEDAUTHOR_H
#include "Common.h"

class cEmbedAuthor final {
private:
	std::string m_name, m_url, m_icon_url, m_proxy_icon_url;

	template<kw::key... Keys, typename T>
	cEmbedAuthor(T&& name, kw::pack<Keys...> pack):
		m_name(std::forward<T>(name)),
		m_url(std::move(kw::get<"url">(pack))),
		m_icon_url(std::move(kw::get<"icon_url">(pack))) {}

public:
	explicit cEmbedAuthor(const json::value&);
	template<kw::key... Keys, typename Str = std::string>
	requires std::constructible_from<std::string, Str&&>
	cEmbedAuthor(Str&& name, kw::arg<Keys>&... kwargs) : cEmbedAuthor(std::forward<Str>(name), kw::pack{ kwargs... }) {}
	/* Getters */
	std::string_view GetName()         const noexcept { return m_name;           }
	std::string_view GetUrl()          const noexcept { return m_url;            }
	std::string_view GetIconUrl()      const noexcept { return m_icon_url;       }
	std::string_view GetProxyIconUrl() const noexcept { return m_proxy_icon_url; }
	/* Movers */
	std::string MoveName()         noexcept { return std::move(m_name);           }
	std::string MoveUrl()          noexcept { return std::move(m_url);            }
	std::string MoveIconUrl()      noexcept { return std::move(m_icon_url);       }
	std::string MoveProxyIconUrl() noexcept { return std::move(m_proxy_icon_url); }
	/* Setters */
	template<typename Str = std::string> requires std::assignable_from<std::string&, Str&&>
	cEmbedAuthor& SetName(Str&& arg) {
		m_name = std::forward<Str>(arg);
		return *this;
	}
	template<typename Str = std::string> requires std::assignable_from<std::string&, Str&&>
	cEmbedAuthor& SetUrl(Str&& arg) {
		m_url = std::forward<Str>(arg);
		return *this;
	}
	template<typename Str = std::string> requires std::assignable_from<std::string&, Str&&>
	cEmbedAuthor& SetIconUrl(Str&& arg) {
		m_icon_url = std::forward<Str>(arg);
		m_proxy_icon_url.clear();
		return *this;
	}
	/* Deleters */
	cEmbedAuthor& ClearUrl() noexcept {
		m_url.clear();
		return *this;
	}
	cEmbedAuthor& ClearIconUrl() noexcept {
		m_icon_url.clear();
		m_proxy_icon_url.clear();
		return *this;
	}
};
typedef   hHandle<cEmbedAuthor>   hEmbedAuthor;
typedef  chHandle<cEmbedAuthor>  chEmbedAuthor;
typedef  uhHandle<cEmbedAuthor>  uhEmbedAuthor;
typedef uchHandle<cEmbedAuthor> uchEmbedAuthor;

void tag_invoke(const json::value_from_tag&, json::value&, const cEmbedAuthor&);
#endif