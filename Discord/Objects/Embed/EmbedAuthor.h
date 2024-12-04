#ifndef DISCORD_EMBEDAUTHOR_H
#define DISCORD_EMBEDAUTHOR_H
#include "Base.h"
/* ================================================================================================================== */
class cEmbedAuthor final {
	std::string m_name, m_url, m_icon_url, m_proxy_icon_url;

public:
	explicit cEmbedAuthor(const boost::json::value&);
	explicit cEmbedAuthor(const boost::json::object&);
	/* Constructor */
	template<iExplicitlyConvertibleTo<std::string> Str = std::string>
	explicit cEmbedAuthor(Str&& name) : m_name(std::forward<Str>(name)) {}
	/* Getters */
	std::string_view         GetName() const noexcept { return m_name;           }
	std::string_view          GetUrl() const noexcept { return m_url;            }
	std::string_view      GetIconUrl() const noexcept { return m_icon_url;       }
	std::string_view GetProxyIconUrl() const noexcept { return m_proxy_icon_url; }
	/* Movers */
	std::string         MoveName() noexcept { return std::move(m_name);           }
	std::string          MoveUrl() noexcept { return std::move(m_url);            }
	std::string      MoveIconUrl() noexcept { return std::move(m_icon_url);       }
	std::string MoveProxyIconUrl() noexcept { return std::move(m_proxy_icon_url); }
	/* Resetters */
	void     ResetUrl() noexcept { m_url.clear();      }
	void ResetIconUrl() noexcept { m_icon_url.clear(); }
	/* Emplacers */
	std::string& EmplaceName() noexcept {
		m_name.clear();
		return m_name;
	}
	std::string& EmplaceUrl() noexcept {
		m_url.clear();
		return m_url;
	}
	std::string& EmplaceIconUrl() noexcept {
		m_icon_url.clear();
		return m_icon_url;
	}
	template<typename Arg = std::string, typename... Args> requires std::constructible_from<std::string, Arg&&, Args&&...>
	std::string& EmplaceName(Arg&& arg, Args&&... args) {
		if constexpr (std::assignable_from<std::string&, Arg&&> && sizeof...(args) == 0)
			return m_name = std::forward<Arg>(arg);
		else
			return m_name = std::string(std::forward<Arg>(arg), std::forward<Args>(args)...);
	}
	template<typename Arg = std::string, typename... Args> requires std::constructible_from<std::string, Arg&&, Args&&...>
	std::string& EmplaceUrl(Arg&& arg, Args&&... args) {
		if constexpr (std::assignable_from<std::string&, Arg&&> && sizeof...(args) == 0)
			return m_url = std::forward<Arg>(arg);
		else
			return m_url = std::string(std::forward<Arg>(arg), std::forward<Args>(args)...);
	}
	template<typename Arg = std::string, typename... Args> requires std::constructible_from<std::string, Arg&&, Args&&...>
	std::string& EmplaceIconUrl(Arg&& arg, Args&&... args) {
		if constexpr (std::assignable_from<std::string&, Arg&&> && sizeof...(args) == 0)
			return m_icon_url = std::forward<Arg>(arg);
		else
			return m_icon_url = std::string(std::forward<Arg>(arg), std::forward<Args>(args)...);
	}
	/* Setters */
	template<iMutable Self, iExplicitlyConvertibleTo<std::string> Arg = std::string>
	Self&& SetName(this Self&& self, Arg&& arg) {
		self.EmplaceName(std::forward<Arg>(arg));
		return std::forward<Self>(self);
	}
	template<iMutable Self, iExplicitlyConvertibleTo<std::string> Arg = std::string>
	Self&& SetUrl(this Self&& self, Arg&& arg) {
		self.EmplaceUrl(std::forward<Arg>(arg));
		return std::forward<Self>(self);
	}
	template<iMutable Self, iExplicitlyConvertibleTo<std::string> Arg = std::string>
	Self&& SetIconUrl(this Self&& self, Arg&& arg) {
		self.EmplaceIconUrl(std::forward<Arg>(arg));
		return std::forward<Self>(self);
	}
};
/* ================================================================================================================== */
cEmbedAuthor
tag_invoke(boost::json::value_to_tag<cEmbedAuthor>, const boost::json::value&);
void
tag_invoke(boost::json::value_from_tag, boost::json::value&, const cEmbedAuthor&);
#endif /* DISCORD_EMBEDAUTHOR_H */