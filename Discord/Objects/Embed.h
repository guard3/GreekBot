#ifndef GREEKBOT_EMBED_H
#define GREEKBOT_EMBED_H
#include "Embed/EmbedAuthor.h"
#include "Embed/EmbedField.h"
#include "Embed/EmbedFooter.h"
#include "Embed/EmbedMedia.h"
#include <span>
#include <vector>

KW_DECLARE(thumbnail, cEmbedMedia)
KW_DECLARE(image, cEmbedMedia)
KW_DECLARE(footer, cEmbedFooter)
KW_DECLARE(author, cEmbedAuthor)
KW_DECLARE(fields, std::vector<cEmbedField>)

/* ================================================================================================= */
class cEmbed final {
private:
	cColor                      m_color;
	std::string                 m_title;
	std::string                 m_description;
	std::string                 m_url;
	std::string                 m_timestamp;
	std::vector<cEmbedField>    m_fields;
	std::optional<cEmbedMedia>  m_thumbnail;
	std::optional<cEmbedMedia>  m_image;
	std::optional<cEmbedMedia>  m_video;
	std::optional<cEmbedFooter> m_footer;
	std::optional<cEmbedAuthor> m_author;

	template<kw::key... Keys>
	cEmbed(kw::pack<Keys...> pack):
		m_color(kw::get<"color">(pack)),
		m_title(std::move(kw::get<"title">(pack))),
		m_description(std::move(kw::get<"description">(pack))),
		m_url(std::move(kw::get<"url">(pack))),
		m_timestamp(std::move(kw::get<"timestamp">(pack))),
		m_fields(std::move(kw::get<"fields">(pack))) {
		if (auto p = kw::get_if<"thumbnail">(pack, kw::nullarg); p)
			m_thumbnail.emplace(std::move(*p));
		if (auto p = kw::get_if<"image">(pack, kw::nullarg); p)
			m_image.emplace(std::move(*p));
		if (auto p = kw::get_if<"footer">(pack, kw::nullarg); p)
			m_footer.emplace(std::move(*p));
		if (auto p = kw::get_if<"author">(pack, kw::nullarg); p)
			m_author.emplace(std::move(*p));
	}

public:
	explicit cEmbed(const json::value&);
	template<kw::key... Keys>
	explicit cEmbed(kw::arg<Keys>&... kwargs) : cEmbed(kw::pack{ kwargs...}) {}
	/* Getters */
	cColor           GetColor()       const noexcept { return m_color;           }
	std::string_view GetTitle()       const noexcept { return m_title;           }
	std::string_view GetDescription() const noexcept { return m_description;     }
	std::string_view GetUrl()         const noexcept { return m_url;             }
	std::string_view GetTimestamp()   const noexcept { return m_timestamp;       }
	std::span<const cEmbedField> GetFields() const noexcept { return m_fields; }
	std::span<      cEmbedField> GetFields()       noexcept { return m_fields; }
	chEmbedMedia  GetThumbnail() const noexcept { return m_thumbnail ? &*m_thumbnail : nullptr; }
	hEmbedMedia   GetThumbnail()       noexcept { return m_thumbnail ? &*m_thumbnail : nullptr; }
	chEmbedMedia  GetImage() const noexcept { return m_image ? &*m_image : nullptr; }
	hEmbedMedia   GetImage()       noexcept { return m_image ? &*m_image : nullptr; }
	chEmbedMedia  GetVideo() const noexcept { return m_video ? &*m_video : nullptr; }
	hEmbedMedia   GetVideo()       noexcept { return m_video ? &*m_video : nullptr; }
	chEmbedFooter GetFooter() const noexcept { return m_footer ? &*m_footer : nullptr; }
	hEmbedFooter  GetFooter()       noexcept { return m_footer ? &*m_footer : nullptr; }
	chEmbedAuthor GetAuthor() const noexcept { return m_author ? &*m_author : nullptr; }
	hEmbedAuthor  GetAuthor()       noexcept { return m_author ? &*m_author : nullptr; }
	/* Setters */
	cEmbed& SetColor(cColor c) {
		m_color = c;
		return *this;
	}
	template<typename Str = std::string> requires std::assignable_from<std::string&, Str&&>
	cEmbed& SetTitle(Str&& arg) {
		m_title = std::forward<Str>(arg);
		return *this;
	}
	template<typename Str = std::string> requires std::assignable_from<std::string&, Str&&>
	cEmbed& SetDescription(Str&& arg) {
		m_description = std::forward<Str>(arg);
		return *this;
	}
	template<typename Str = std::string> requires std::assignable_from<std::string&, Str&&>
	cEmbed& SetUrl(Str&& arg) {
		m_url = std::forward<Str>(arg);
		return *this;
	}
	template<typename Str = std::string> requires std::assignable_from<std::string&, Str&&>
	cEmbed& SetTimestamp(Str&& arg) {
		m_timestamp = std::forward<Str>(arg);
		return *this;
	}
	template<typename Arg = cEmbedMedia, typename... Args> requires std::constructible_from<cEmbedMedia, Arg&&, Args&&...>
	cEmbed& SetThumbnail(Arg&& arg, Args&&... args) {
		m_thumbnail.emplace(std::forward<Arg>(arg), std::forward<Args>(args)...);
		return *this;
	}
	template<typename Arg = cEmbedMedia, typename... Args> requires std::constructible_from<cEmbedMedia, Arg&&, Args&&...>
	cEmbed& SetImage(Arg&& arg, Args&&... args) {
		m_image.emplace(std::forward<Arg>(arg), std::forward<Args>(args)...);
		return *this;
	}
	template<typename Arg = cEmbedFooter, typename... Args> requires std::constructible_from<cEmbedFooter, Arg&&, Args&&...>
	cEmbed& SetFooter(Arg&& arg, Args&&... args) {
		m_footer.emplace(std::forward<Arg>(arg), std::forward<Args>(args)...);
		return *this;
	}
	template<typename Arg = cEmbedAuthor, typename... Args> requires std::constructible_from<cEmbedAuthor, Arg&&, Args&&...>
	cEmbed& SetAuthor(Arg&& arg, Args&&... args) {
		m_author.emplace(std::forward<Arg>(arg), std::forward<Args>(args)...);
		return *this;
	}
	template<typename Arg = std::vector<cEmbedField>> requires std::assignable_from<std::vector<cEmbedField>&, Arg&&>
	cEmbed& SetFields(Arg&& arg) {
		m_fields = std::forward<Arg>(arg);
		return *this;
	}
	template<typename Arg = cEmbedField, typename... Args> requires std::constructible_from<cEmbedField, Arg&&, Args&&...>
	cEmbed& AddField(Arg&& arg, Args&&... args) {
		m_fields.emplace_back(std::forward<Arg>(arg), std::forward<Args>(args)...);
		return *this;
	}
	/* Deleters */
	cEmbed& ClearTitle() noexcept {
		m_title.clear();
		return *this;
	}
	cEmbed& ClearDescription() noexcept {
		m_description.clear();
		return *this;
	}
	cEmbed& ClearUrl() noexcept {
		m_url.clear();
		return *this;
	}
	cEmbed& ClearTimestamp() noexcept {
		m_timestamp.clear();
		return *this;
	}
	cEmbed& ClearThumbnail() noexcept {
		m_thumbnail.reset();
		return *this;
	}
	cEmbed& ClearImage() noexcept {
		m_image.reset();
		return *this;
	}
	cEmbed& ClearFooter() noexcept {
		m_footer.reset();
		return *this;
	}
	cEmbed& ClearAuthor() noexcept {
		m_author.reset();
		return *this;
	}
	cEmbed& ClearFields() noexcept {
		m_fields.clear();
		return *this;
	}
};
typedef   hHandle<cEmbed>   hEmbed;
typedef  chHandle<cEmbed>  chEmbed;
typedef  uhHandle<cEmbed>  uhEmbed;
typedef uchHandle<cEmbed> uchEmbed;

cEmbedField tag_invoke(json::value_to_tag<cEmbedField>, const json::value&);
cEmbed tag_invoke(boost::json::value_to_tag<cEmbed>, const boost::json::value&);
void tag_invoke(const json::value_from_tag&, json::value&, const cEmbedField&);
void tag_invoke(const json::value_from_tag&, json::value&, const cEmbed&);

KW_DECLARE(embeds, std::vector<cEmbed>)
#endif // GREEKBOT_EMBED_H