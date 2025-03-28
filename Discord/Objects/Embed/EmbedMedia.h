#ifndef DISCORD_EMBEDMEDIA_H
#define DISCORD_EMBEDMEDIA_H
#include "Base.h"
/* ================================================================================================================== */
class cEmbedMedia final {
	std::string m_url;       // The source url of the image or video
	std::string m_proxy_url; // A proxied url of the image or video
	int         m_width;
	int         m_height;

public:
	explicit cEmbedMedia(const boost::json::value&);
	explicit cEmbedMedia(const boost::json::object&);
	/* Constructor */
	template<iExplicitlyConvertibleTo<std::string> Str = std::string>
	explicit cEmbedMedia(Str&& url) : m_url(std::forward<Str>(url)), m_width(-1), m_height(-1) {}
	/* Getters */
	std::string_view      GetUrl() const noexcept { return m_url;       }
	std::string_view GetProxyUrl() const noexcept { return m_proxy_url; }
	int                 GetWidth() const noexcept { return m_width;     }
	int                GetHeight() const noexcept { return m_height;    }
	/* Movers */
	std::string      MoveUrl() noexcept { return std::move(m_url);       }
	std::string MoveProxyUrl() noexcept { return std::move(m_proxy_url); }
	/* Emplacers */
	std::string& EmplaceUrl() noexcept {
		m_url.clear();
		return m_url;
	}
	template<typename Arg = std::string, typename... Args> requires std::constructible_from<std::string, Arg&&, Args&&...>
	std::string& EmplaceUrl(Arg&& arg, Args&&... args) {
		if constexpr (std::assignable_from<std::string&, Arg&&> && sizeof...(args) == 0)
			return m_url = std::forward<Arg>(arg);
		else
			return m_url = std::string(std::forward<Arg>(arg), std::forward<Args>(args)...);
	}
	/* Setters */
	template<iMutable Self, iExplicitlyConvertibleTo<std::string> Arg = std::string>
	Self&& SetUrl(this Self&& self, Arg&& arg)  {
		self.EmplaceUrl(std::forward<Arg>(arg));
		return std::forward<Self>(self);
	}
};
/* ================================================================================================================== */
cEmbedMedia
tag_invoke(boost::json::value_to_tag<cEmbedMedia>, const boost::json::value&);
void
tag_invoke(boost::json::value_from_tag, boost::json::value&, const cEmbedMedia&);
#endif /* DISCORD_EMBEDMEDIA_H */