#ifndef DISCORD_EMBED_H
#define DISCORD_EMBED_H
#include "Embed/EmbedAuthor.h"
#include "Embed/EmbedField.h"
#include "Embed/EmbedFooter.h"
#include "Embed/EmbedMedia.h"
#include "EmbedFwd.h"
#include <chrono>
#include <optional>
#include <span>
#include <vector>
/* ================================================================================================================== */
class cEmbed final {
	cColor                      m_color;
	std::string                 m_title;
	std::string                 m_description;
	std::string                 m_url;
	std::chrono::sys_time<std::chrono::milliseconds> m_timestamp;
	std::vector<cEmbedField>    m_fields;
	std::optional<cEmbedMedia>  m_thumbnail;
	std::optional<cEmbedMedia>  m_image;
	std::optional<cEmbedMedia>  m_video;
	std::optional<cEmbedFooter> m_footer;
	std::optional<cEmbedAuthor> m_author;

public:
	cEmbed() = default;
	explicit cEmbed(const boost::json::value&);
	explicit cEmbed(const boost::json::object&);
	/* Getters */
	cColor                 GetColor() const noexcept { return m_color;       }
	std::string_view       GetTitle() const noexcept { return m_title;       }
	std::string_view GetDescription() const noexcept { return m_description; }
	std::string_view         GetUrl() const noexcept { return m_url;         }
	auto               GetTimestamp() const noexcept { return m_timestamp;   }
	auto    GetFields(this auto& self) noexcept { return std::span(self.m_fields); }
	auto GetThumbnail(this auto& self) noexcept { return cPtr(self.m_thumbnail ? self.m_thumbnail.operator->() : nullptr); }
	auto     GetImage(this auto& self) noexcept { return cPtr(self.m_image     ? self.m_image.operator->()     : nullptr); }
	auto     GetVideo(this auto& self) noexcept { return cPtr(self.m_video     ? self.m_video.operator->()     : nullptr); }
	auto    GetFooter(this auto& self) noexcept { return cPtr(self.m_footer    ? self.m_footer.operator->()    : nullptr); }
	auto    GetAuthor(this auto& self) noexcept { return cPtr(self.m_author    ? self.m_author.operator->()    : nullptr); }
	/* Movers */
	std::string                    MoveTitle() noexcept { return std::move(m_title);       }
	std::string              MoveDescription() noexcept { return std::move(m_description); }
	std::string                      MoveUrl() noexcept { return std::move(m_url);         }
	std::optional<cEmbedMedia> MoveThumbnail() noexcept { return std::move(m_thumbnail);   }
	std::optional<cEmbedMedia>     MoveImage() noexcept { return std::move(m_image);       }
	std::optional<cEmbedMedia>     MoveVideo() noexcept { return std::move(m_video);       }
	std::optional<cEmbedFooter>   MoveFooter() noexcept { return std::move(m_footer);      }
	std::optional<cEmbedAuthor>   MoveAuthor() noexcept { return std::move(m_author);      }
	std::vector<cEmbedField>      MoveFields() noexcept { return std::move(m_fields);      }
	/* Resetters */
	void       ResetColor() noexcept { m_color = {};          }
	void       ResetTitle() noexcept { m_title.clear();       }
	void ResetDescription() noexcept { m_description.clear(); }
	void         ResetUrl() noexcept { m_url.clear();         }
	void   ResetThumbnail() noexcept { m_thumbnail.reset();   }
	void       ResetImage() noexcept { m_image.reset();       }
	void      ResetFooter() noexcept { m_footer.reset();      }
	void      ResetAuthor() noexcept { m_author.reset();      }
	void      ResetFields() noexcept { m_fields.clear();      }
	void   ResetTimestamp() noexcept { m_timestamp = {};      }
	/* Emplacers */
	cColor& EmplaceColor() noexcept {
		return m_color = {};
	}
	std::string& EmplaceTitle() noexcept {
		m_title.clear();
		return m_title;
	}
	std::string& EmplaceDescription() noexcept {
		m_description.clear();
		return m_description;
	}
	std::string& EmplaceUrl() noexcept {
		m_url.clear();
		return m_url;
	}
	std::vector<cEmbedField>& EmplaceFields() noexcept {
		m_fields.clear();
		return m_fields;
	}
	template<typename Arg = cColor, typename... Args> requires std::constructible_from<cColor, Arg&&, Args&&...>
	cColor& EmplaceColor(Arg&& arg, Args&&... args) noexcept {
		if constexpr (std::assignable_from<cColor&, Arg&&> && sizeof...(args) == 0)
			return m_color = std::forward<Arg>(arg);
		else
			return m_color = cColor(std::forward<Arg>(arg), std::forward<Args>(args)...);
	}
	template<typename Arg = std::string, typename... Args> requires std::constructible_from<std::string, Arg&&, Args&&...>
	std::string& EmplaceTitle(Arg&& arg, Args&&... args) {
		if constexpr (std::assignable_from<std::string&, Arg&&> && sizeof...(args) == 0)
			return m_title = std::forward<Arg>(arg);
		else
			return m_title = std::string(std::forward<Arg>(arg), std::forward<Args>(args)...);
	}
	template<typename Arg = std::string, typename... Args> requires std::constructible_from<std::string, Arg&&, Args&&...>
	std::string& EmplaceDescription(Arg&& arg, Args&&... args) {
		if constexpr (std::assignable_from<std::string&, Arg&&> && sizeof...(args) == 0)
			return m_description = std::forward<Arg>(arg);
		else
			return m_description = std::string(std::forward<Arg>(arg), std::forward<Args>(args)...);
	}
	template<typename Arg = std::string, typename... Args> requires std::constructible_from<std::string, Arg&&, Args&&...>
	std::string& EmplaceUrl(Arg&& arg, Args&&... args) {
		if constexpr (std::assignable_from<std::string&, Arg&&> && sizeof...(args) == 0)
			return m_url = std::forward<Arg>(arg);
		else
			return m_url = std::string(std::forward<Arg>(arg), std::forward<Args>(args)...);
	}
	template<typename Arg = cEmbedMedia> requires std::constructible_from<cEmbedMedia, Arg&&>
	cEmbedMedia& EmplaceThumbnail(Arg&& arg) {
		if constexpr (std::assignable_from<cEmbedMedia&, Arg&&>)
			return *(m_thumbnail = std::forward<Arg>(arg));
		else
			return m_thumbnail.emplace(std::forward<Arg>(arg));
	}
	template<typename Arg = cEmbedMedia> requires std::constructible_from<cEmbedMedia, Arg&&>
	cEmbedMedia& EmplaceImage(Arg&& arg) {
		if constexpr (std::assignable_from<cEmbedMedia&, Arg&&>)
			return *(m_image = std::forward<Arg>(arg));
		else
			return m_image.emplace(std::forward<Arg>(arg));
	}
	template<typename Arg = cEmbedFooter, typename... Args> requires std::constructible_from<cEmbedFooter, Arg&&, Args&&...>
	cEmbedFooter& EmplaceFooter(Arg&& arg, Args&&... args) {
		if constexpr (std::assignable_from<cEmbedFooter&, Arg&&> && sizeof...(args) == 0)
			return *(m_footer = std::forward<Arg>(arg));
		else
			return m_footer.emplace(std::forward<Arg>(arg), std::forward<Args>(args)...);
	}
	template<typename Arg = cEmbedAuthor> requires std::constructible_from<cEmbedAuthor, Arg&&>
	cEmbedAuthor& EmplaceAuthor(Arg&& arg) {
		if constexpr (std::assignable_from<cEmbedAuthor&, Arg&&>)
			return *(m_author = std::forward<Arg>(arg));
		else
			return m_author.emplace(std::forward<Arg>(arg));
	}
	template<typename Arg = std::vector<cEmbedField>, typename... Args> requires std::constructible_from<std::vector<cEmbedField>, Arg&&, Args&&...>
	std::vector<cEmbedField>& EmplaceFields(Arg&& arg, Args&&... args) {
		if constexpr (std::assignable_from<std::vector<cEmbedField>&, Arg&&> && sizeof...(args) == 0)
			return m_fields = std::forward<Arg>(arg);
		else
			return m_fields = std::vector<cEmbedField>(std::forward<Arg>(arg), std::forward<Args>(args)...);
	}
	/* Setters */
	template<iMutable Self>
	Self&& SetColor(this Self&& self, cColor c) noexcept {
		self.m_color = c;
		return std::forward<Self>(self);
	}
	template<iMutable Self, iExplicitlyConvertibleTo<std::string> Arg = std::string>
	Self&& SetTitle(this Self&& self, Arg&& arg) {
		self.EmplaceTitle(std::forward<Arg>(arg));
		return std::forward<Self>(self);
	}
	template<iMutable Self, iExplicitlyConvertibleTo<std::string> Arg = std::string>
	Self&& SetDescription(this Self&& self, Arg&& arg) {
		self.EmplaceDescription(std::forward<Arg>(arg));
		return std::forward<Self>(self);
	}
	template<iMutable Self, iExplicitlyConvertibleTo<std::string> Arg = std::string>
	Self&& SetUrl(this Self&& self, Arg&& arg) {
		self.EmplaceUrl(std::forward<Arg>(arg));
		return std::forward<Self>(self);
	}
	template<iMutable Self, iExplicitlyConvertibleTo<cEmbedMedia> Arg = cEmbedMedia>
	Self&& SetThumbnail(this Self&& self, Arg&& arg) {
		self.EmplaceThumbnail(std::forward<Arg>(arg));
		return std::forward<Self>(self);
	}
	template<iMutable Self, iExplicitlyConvertibleTo<cEmbedMedia> Arg = cEmbedMedia>
	Self&& SetImage(this Self&& self, Arg&& arg) {
		self.EmplaceImage(std::forward<Arg>(arg));
		return std::forward<Self>(self);
	}
	template<iMutable Self, iExplicitlyConvertibleTo<cEmbedFooter> Arg = cEmbedFooter>
	Self&& SetFooter(this Self&& self, Arg&& arg) {
		self.EmplaceFooter(std::forward<Arg>(arg));
		return std::forward<Self>(self);
	}
	template<iMutable Self, iExplicitlyConvertibleTo<cEmbedAuthor> Arg = cEmbedAuthor>
	Self&& SetAuthor(this Self&& self, Arg&& arg) {
		self.EmplaceAuthor(std::forward<Arg>(arg));
		return std::forward<Self>(self);
	}
	template<iMutable Self, iExplicitlyConvertibleTo<std::vector<cEmbedField>> Arg = std::vector<cEmbedField>>
	Self&& SetFields(this Self&& self, Arg&& arg) {
		self.EmplaceFields(std::forward<Arg>(arg));
		return std::forward<Self>(self);
	}
	template<iMutable Self>
	Self&& SetTimestamp(this Self&& self, std::chrono::sys_time<std::chrono::milliseconds> arg) noexcept {
		self.m_timestamp = arg;
		return std::forward<Self>(self);
	}
};
/* ================================================================================================================== */
cEmbed
tag_invoke(boost::json::value_to_tag<cEmbed>, const boost::json::value&);
void
tag_invoke(boost::json::value_from_tag, boost::json::value&, const cEmbed&);
#endif /* DISCORD_EMBED_H */