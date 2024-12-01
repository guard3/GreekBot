#ifndef DISCORD_EMBEDFOOTER_H
#define DISCORD_EMBEDFOOTER_H
#include "Base.h"

class cEmbedFooter final {
	std::string m_text, m_icon_url, m_proxy_icon_url;

public:
	explicit cEmbedFooter(const boost::json::value&);
	explicit cEmbedFooter(const boost::json::object&);

	template<typename Str1 = std::string, typename Str2 = std::string> requires requires {
		requires std::constructible_from<std::string, Str1&&>;
		requires std::constructible_from<std::string, Str2&&>;
	} explicit cEmbedFooter(Str1&& text, Str2&& icon_url = {}) : m_text(std::forward<Str1>(text)), m_icon_url(std::forward<Str2>(icon_url)) {}
	/* Getters */
	std::string_view         GetText() const noexcept { return m_text;           }
	std::string_view      GetIconUrl() const noexcept { return m_icon_url;       }
	std::string_view GetProxyIconUrl() const noexcept { return m_proxy_icon_url; }
	/* Movers */
	std::string         MoveText() noexcept { return std::move(m_text);           }
	std::string      MoveIconUrl() noexcept { return std::move(m_icon_url);       }
	std::string MoveProxyIconUrl() noexcept { return std::move(m_proxy_icon_url); }
	/* Resetters */
	void ResetIconUrl() noexcept { m_icon_url.clear(); }
	/* Emplacers */
	std::string& EmplaceText() noexcept {
		m_text.clear();
		return m_text;
	}
	std::string& EmplaceIconUrl() noexcept {
		m_icon_url.clear();
		return m_icon_url;
	}
	template<typename Arg = std::string, typename... Args> requires std::constructible_from<std::string, Arg&&, Args&&...>
	std::string& EmplaceText(Arg&& arg, Args&&... args) {
		if constexpr (std::assignable_from<std::string&, Arg&&> && sizeof...(args) == 0)
			return m_text = std::forward<Arg>(arg);
		else
			return m_text = std::string(std::forward<Arg>(arg), std::forward<Args>(arg)...);
	}
	template<typename Arg = std::string, typename... Args> requires std::constructible_from<std::string, Arg&&, Args&&...>
	std::string& EmplaceIconUrl(Arg&& arg, Args&&... args) noexcept {
		if constexpr (std::assignable_from<std::string&, Arg&&> && sizeof...(args) == 0)
			return m_icon_url = std::forward<Arg>(arg);
		else
			return m_icon_url = std::string(std::forward<Arg>(arg), std::forward<Args>(args)...);
	}
	/* Setters */
	template<typename Arg = std::string> requires std::constructible_from<std::string, Arg&&>
	cEmbedFooter& SetText(Arg&& arg) & {
		EmplaceText(std::forward<Arg>(arg));
		return *this;
	}
	template<typename Arg = std::string> requires std::constructible_from<std::string, Arg&&>
	cEmbedFooter& SetIconUrl(Arg&& arg) & {
		EmplaceIconUrl(std::forward<Arg>(arg));
		return *this;
	}
	template<typename Arg = std::string> requires std::constructible_from<std::string, Arg&&>
	cEmbedFooter&& SetText(Arg&& arg) && { return std::move(std::forward<Arg>(arg)); }
	template<typename Arg = std::string> requires std::constructible_from<std::string, Arg&&>
	cEmbedFooter&& SetIconUrl(Arg&& arg) && { return std::move(std::forward<Arg>(arg)); }
};
typedef   hHandle<cEmbedFooter>   hEmbedFooter;
typedef  chHandle<cEmbedFooter>  chEmbedFooter;
typedef  uhHandle<cEmbedFooter>  uhEmbedFooter;
typedef uchHandle<cEmbedFooter> uchEmbedFooter;

cEmbedFooter
tag_invoke(boost::json::value_to_tag<cEmbedFooter>, const boost::json::value&);
void
tag_invoke(boost::json::value_from_tag, boost::json::value&, const cEmbedFooter&);
#endif /* DISCORD_EMBEDFOOTER_H */