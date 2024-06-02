#ifndef DISCORD_EMBED_H
#define DISCORD_EMBED_H
#include "Embed/EmbedAuthor.h"
#include "Embed/EmbedField.h"
#include "Embed/EmbedFooter.h"
#include "Embed/EmbedMedia.h"
#include <optional>
#include <span>
#include <vector>
/* ================================================================================================= */
class cEmbed final {
private:
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
	// TODO: are these helpers really necessary??
	template<typename Var, typename Arg>
	static void set_var(Var& var, Arg&& arg) {
		if constexpr (std::assignable_from<Var&, Arg&&>) {
			var = std::forward<Arg>(arg);
		} else if constexpr (std::is_nothrow_default_constructible_v<Var>) try {
			std::destroy_at(std::addressof(var));
			std::construct_at(std::addressof(var), std::forward<Arg>(arg));
		} catch (...) {
			std::construct_at(std::addressof(var));
			throw;
		} else {
			var = Var(arg);
		}
	}
	template<typename Var, typename Arg>
	static void set_var(std::optional<Var>& var, Arg&& arg) {
		if constexpr (std::assignable_from<Var&, Arg&&>) {
			var = std::forward<Arg>(arg);
		} else {
			var.emplace(std::forward<Arg>(arg));
		}
	}

	template<typename Var, typename Arg, typename... Args>
	static Var& emplace_var(Var& var, Arg&& arg, Args&&... args) {
		if constexpr (sizeof...(args) == 0) {
			set_var(var, std::forward<Arg>);
			return var;
		} else if constexpr (std::is_nothrow_default_constructible_v<Var>) try {
			std::destroy_at(std::addressof(var));
			return *std::construct_at(std::addressof(var), std::forward<Arg>(arg), std::forward<Args>(args)...);
		} catch (...) {
			std::construct_at(std::addressof(var));
			throw;
		} else {
			return var = Var(std::forward<Arg>(arg), std::forward<Args>(args)...);
		}
	}

	template<typename Var, typename Arg, typename... Args>
	static Var& emplace_var(std::optional<Var>& opt, Arg&& arg, Args&&... args) {
		if constexpr (sizeof...(args) == 0) {
			set_var(opt, std::forward<Arg>(arg));
			return *opt;
		} else {
			return opt.emplace(std::forward<Arg>, std::forward<Args>(args)...);
		}
	}


public:
	explicit cEmbed(const json::value&);
	explicit cEmbed(const json::object&);
	cEmbed() = default;
	/* Getters */
	cColor                 GetColor() const noexcept { return m_color;       }
	std::string_view       GetTitle() const noexcept { return m_title;       }
	std::string_view GetDescription() const noexcept { return m_description; }
	std::string_view         GetUrl() const noexcept { return m_url;         }
	chEmbedMedia       GetThumbnail() const noexcept { return m_thumbnail ? m_thumbnail.operator->() : nullptr; }
	 hEmbedMedia       GetThumbnail()       noexcept { return m_thumbnail ? m_thumbnail.operator->() : nullptr; }
	chEmbedMedia           GetImage() const noexcept { return m_image     ?     m_image.operator->() : nullptr; }
	 hEmbedMedia           GetImage()       noexcept { return m_image     ?     m_image.operator->() : nullptr; }
	chEmbedMedia           GetVideo() const noexcept { return m_video     ?     m_video.operator->() : nullptr; }
	 hEmbedMedia           GetVideo()       noexcept { return m_video     ?     m_video.operator->() : nullptr; }
	chEmbedFooter         GetFooter() const noexcept { return m_footer    ?    m_footer.operator->() : nullptr; }
	 hEmbedFooter         GetFooter()       noexcept { return m_footer    ?    m_footer.operator->() : nullptr; }
	chEmbedAuthor         GetAuthor() const noexcept { return m_author    ?    m_author.operator->() : nullptr; }
	 hEmbedAuthor         GetAuthor()       noexcept { return m_author    ?    m_author.operator->() : nullptr; }
	std::span<const cEmbedField> GetFields() const noexcept { return m_fields; }
	std::span<      cEmbedField> GetFields()       noexcept { return m_fields; }
	template<typename Duration = std::chrono::milliseconds>
	std::chrono::sys_time<Duration> GetTimestamp() const noexcept {
		return std::chrono::floor<Duration>(m_timestamp);
	}
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
	/* Setters */
	cEmbed& SetColor(cColor c) {
		m_color = c;
		return *this;
	}
	template<typename Arg = std::string> requires std::constructible_from<std::string, Arg&&>
	cEmbed& SetTitle(Arg&& arg) {
		set_var(m_title, std::forward<Arg>(arg));
		return *this;
	}
	template<typename Arg = std::string> requires std::constructible_from<std::string, Arg&&>
	cEmbed& SetDescription(Arg&& arg) {
		set_var(m_description, std::forward<Arg>(arg));
		return *this;
	}
	template<typename Arg = std::string> requires std::constructible_from<std::string, Arg&&>
	cEmbed& SetUrl(Arg&& arg) {
		set_var(m_url, std::forward<Arg>(arg));
		return *this;
	}
	cEmbed& SetTimestamp(std::chrono::sys_time<std::chrono::milliseconds> arg) {
		m_timestamp = arg;
		return *this;
	}
	template<typename Arg = cEmbedMedia> requires std::constructible_from<cEmbedMedia, Arg&&>
	cEmbed& SetThumbnail(Arg&& arg) {
		set_var(m_thumbnail, std::forward<Arg>(arg));
		return *this;
	}
	template<typename Arg = cEmbedMedia> requires std::constructible_from<cEmbedMedia, Arg&&>
	cEmbed& SetImage(Arg&& arg) {
		set_var(m_image, std::forward<Arg>(arg));
		return *this;
	}
	template<typename Arg = cEmbedFooter> requires std::constructible_from<cEmbedFooter, Arg&&>
	cEmbed& SetFooter(Arg&& arg) {
		set_var(m_footer, std::forward<Arg>(arg));
		return *this;
	}
	template<typename Arg = cEmbedAuthor> requires std::constructible_from<cEmbedAuthor, Arg&&>
	cEmbed& SetAuthor(Arg&& arg) {
		set_var(m_author, std::forward<Arg>(arg));
		return *this;
	}
	template<typename Arg = std::vector<cEmbedField>> requires std::constructible_from<std::vector<cEmbedField>, Arg&&>
	cEmbed& SetFields(Arg&& arg) {
		set_var(m_fields, std::forward<Arg>(arg));
		return *this;
	}
	/* Emplacers */
	cColor& EmplaceColor(cColor color) noexcept {
		return m_color = color;
	}
	std::string& EmplaceTitle() noexcept {
		m_title.clear();
		return m_title;
	}
	template<typename Arg = std::string, typename... Args> requires std::constructible_from<std::string, Arg&&, Args&&...>
	std::string& EmplaceTitle(Arg&& arg, Args&&... args) {
		return emplace_var(m_title, std::forward<Arg>(arg), std::forward<Args>(args)...);
	}
	std::string& EmplaceDescription() noexcept {
		m_description.clear();
		return m_description;
	}
	template<typename Arg = std::string, typename... Args> requires std::constructible_from<std::string, Arg&&, Args&&...>
	std::string& EmplaceDescription(Arg&& arg, Args&&... args) {
		return emplace_var(m_description, std::forward<Arg>(arg), std::forward<Args>(args)...);
	}
	std::string& EmplaceUrl() noexcept {
		m_url.clear();
		return m_url;
	}
	template<typename Arg = std::string, typename... Args> requires std::constructible_from<std::string, Arg&&, Args&&...>
	std::string& EmplaceUrl(Arg&& arg, Args&&... args) {
		return emplace_var(m_url, std::forward<Arg>(arg), std::forward<Args>(args)...);
	}
	template<typename... Args> requires std::constructible_from<cEmbedMedia, Args&&...>
	cEmbedMedia& EmplaceThumbnail(Args&&... args) {
		return m_thumbnail.emplace(std::forward<Args>(args)...);
	}
	template<typename... Args> requires std::constructible_from<cEmbedMedia, Args&&...>
	cEmbedMedia& EmplaceImage(Args&&... args) {
		return m_image.emplace(std::forward<Args>(args)...);
	}
	template<typename... Args> requires std::constructible_from<cEmbedFooter, Args&&...>
	cEmbedFooter& EmplaceFooter(Args&&... args) {
		return m_footer.emplace(std::forward<Args>(args)...);
	}
	template<typename... Args> requires std::constructible_from<cEmbedAuthor, Args&&...>
	cEmbedAuthor& EmplaceAuthor(Args&&... args) {
		return m_author.emplace(std::forward<Args>(args)...);
	}
	std::vector<cEmbedField>& EmplaceFields() noexcept {
		m_fields.clear();
		return m_fields;
	}
	template<typename Arg, typename... Args> requires std::constructible_from<std::vector<cEmbedField>, Arg&&, Args&&...>
	std::vector<cEmbedField>& EmplaceFields(Arg&& arg, Args&&... args) {
		return emplace_var(m_fields, std::forward<Arg>(arg), std::forward<Args>(args)...);
	}
};
typedef   hHandle<cEmbed>   hEmbed;
typedef  chHandle<cEmbed>  chEmbed;
typedef  uhHandle<cEmbed>  uhEmbed;
typedef uchHandle<cEmbed> uchEmbed;

template<>
inline std::chrono::sys_time<std::chrono::milliseconds> cEmbed::GetTimestamp() const noexcept {
	return m_timestamp;
}

cEmbedField tag_invoke(json::value_to_tag<cEmbedField>, const json::value&);
cEmbed tag_invoke(boost::json::value_to_tag<cEmbed>, const boost::json::value&);
void tag_invoke(const json::value_from_tag&, json::value&, const cEmbedField&);
void tag_invoke(const json::value_from_tag&, json::value&, const cEmbed&);
#endif /* DISCORD_EMBED_H */