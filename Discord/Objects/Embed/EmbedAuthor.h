#ifndef GREEKBOT_EMBEDAUTHOR_H
#define GREEKBOT_EMBEDAUTHOR_H
#include "Common.h"

class cEmbedAuthor final {
private:
	std::string m_name, m_url, m_icon_url, m_proxy_icon_url;

	template<iKwArg... KwArgs>
	cEmbedAuthor(std::string name, cKwPack<KwArgs...>&& pack):
		m_name(std::move(name)),
		m_url(KwMove<KW_URL>(pack)),
		m_icon_url(KwMove<KW_ICON_URL>(pack)) {}

public:
	cEmbedAuthor(const json::object&);
	cEmbedAuthor(const json::value&);
	cEmbedAuthor(const char* name) : m_name(name) {}
	template<iKwArg... KwArgs>
	cEmbedAuthor(std::string name, KwArgs&... kwargs) : cEmbedAuthor(std::move(name), cKwPack<KwArgs...>(kwargs...)) {}
	/* Non const getters */
	std::string& GetName()         noexcept { return m_name;           }
	std::string& GetUrl()          noexcept { return m_url;            }
	std::string& GetIconUrl()      noexcept { return m_icon_url;       }
	std::string& GetProxyIconUrl() noexcept { return m_proxy_icon_url; }
	/* Getters */
	const std::string& GetName()         const noexcept { return m_name;           }
	const std::string& GetUrl()          const noexcept { return m_url;            }
	const std::string& GetIconUrl()      const noexcept { return m_icon_url;       }
	const std::string& GetProxyIconUrl() const noexcept { return m_proxy_icon_url; }
	/* Movers */
	std::string MoveName()         noexcept { return std::move(m_name);           }
	std::string MoveUrl()          noexcept { return std::move(m_url);            }
	std::string MoveIconUrl()      noexcept { return std::move(m_icon_url);       }
	std::string MoveProxyIconUrl() noexcept { return std::move(m_proxy_icon_url); }
	/* Setters */
	template<typename Arg, typename... Args>
	cEmbedAuthor& SetName(Arg&& arg, Args&&... args) { m_name = { std::forward<Arg>(arg), std::forward<Args>(args)... }; return *this; }
	template<typename Arg, typename... Args>
	cEmbedAuthor& SetUrl(Arg&& arg, Args&&... args) { m_url = { std::forward<Arg>(arg), std::forward<Args>(args)... }; return *this; }
	template<typename Arg, typename... Args>
	cEmbedAuthor& SetIconUrl(Arg&& arg, Args&&... args) {
		m_icon_url = { std::forward<Arg>(arg), std::forward<Args>(args)... };
		m_proxy_icon_url.clear();
		return *this;
	}
	/* Publish */
	json::object ToJson() const;
};
template<>
inline cEmbedAuthor& cEmbedAuthor::SetUrl<std::nullptr_t>(std::nullptr_t&&) { m_url.clear(); return *this; }
template<>
inline cEmbedAuthor& cEmbedAuthor::SetIconUrl<std::nullptr_t>(std::nullptr_t&&) {
	m_icon_url.clear();
	m_proxy_icon_url.clear();
	return *this;
}
typedef   hHandle<cEmbedAuthor>   hEmbedAuthor;
typedef  chHandle<cEmbedAuthor>  chEmbedAuthor;
typedef  uhHandle<cEmbedAuthor>  uhEmbedAuthor;
typedef uchHandle<cEmbedAuthor> uchEmbedAuthor;
typedef  shHandle<cEmbedAuthor>  shEmbedAuthor;
typedef schHandle<cEmbedAuthor> schEmbedAuthor;
#endif