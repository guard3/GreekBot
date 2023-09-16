#ifndef GREEKBOT_EMBED_H
#define GREEKBOT_EMBED_H
#include "EmbedMedia.h"
#include "EmbedAuthor.h"
#include "EmbedFooter.h"
#include <vector>
#include "Kwarg.h"

template<typename T>
class cWrapper {
private:
	uhHandle<T> m_handle;

public:
	cWrapper() = default;
	template<typename Arg, typename... Args>
	cWrapper(Arg&& arg, Args&&... args) : m_handle(cHandle::MakeUnique<T>(std::forward<Arg>(arg), std::forward<Args>(args)...)) {}
	cWrapper(const cWrapper& o) : m_handle(cHandle::MakeUnique<T>(*o.m_handle)) {}
	cWrapper(cWrapper&&) noexcept = default;

	cWrapper& operator=(const cWrapper& o) {
		if (m_handle)
			*m_handle = *o.m_handle;
		else
			m_handle = cHandle::MakeUnique<T>(*o.m_handle);
		return *this;
	}
	cWrapper& operator=(cWrapper&& o) noexcept {
		m_handle.swap(o.m_handle);
		return *this;
	}

	T* Get() const { return m_handle.get(); }
	uhHandle<T> Move() { return std::move(m_handle); }
};

class cEmbedField final {
private:
	std::string m_name, m_value;
	bool        m_inline;

public:
	cEmbedField(const json::object&);
	cEmbedField(const json::value&);
	cEmbedField(std::string name, std::string value, bool inline_ = false) : m_name(std::move(name)), m_value(std::move(value)), m_inline(inline_) {}
	/* Getters */
	const std::string& GetName()  const noexcept { return m_name;   }
	const std::string& GetValue() const noexcept { return m_value;  }
	bool               IsInline() const noexcept { return m_inline; }
	/* Movers */
	std::string MoveName()  noexcept { return std::move(m_name ); }
	std::string MoveValue() noexcept { return std::move(m_value); }
	/* Setters */
	cEmbedField& SetName(std::string name) {
		m_name = std::move(name);
		return *this;
	}
	cEmbedField& SetValue(std::string value) {
		m_value = std::move(value);
		return *this;
	}
	cEmbedField& SetInline(bool inline_) {
		m_inline = inline_;
		return *this;
	}
};
typedef   hHandle<cEmbedField>   hEmbedField;
typedef  chHandle<cEmbedField>  chEmbedField;
typedef  uhHandle<cEmbedField>  uhEmbedField;
typedef uchHandle<cEmbedField> uchEmbedField;
typedef  shHandle<cEmbedField>  shEmbedField;
typedef schHandle<cEmbedField> schEmbedField;

KW_DECLARE(thumbnail, KW_THUMBNAIL, cWrapper<cEmbedMedia>)
KW_DECLARE(image, KW_IMAGE, cWrapper<cEmbedMedia>)
KW_DECLARE(footer, KW_FOOTER, cWrapper<cEmbedFooter>)
KW_DECLARE(author, KW_AUTHOR, cWrapper<cEmbedAuthor>)
KW_DECLARE(fields, KW_FIELDS, std::vector<cEmbedField>)

/* ================================================================================================= */
class cEmbed final {
private:
	cColor        m_color;
	std::string   m_title, m_description, m_url, m_timestamp;
	uhEmbedMedia  m_thumbnail, m_image, m_video;
	uhEmbedFooter m_footer;
	uhEmbedAuthor m_author;
	std::vector<cEmbedField> m_fields;

	cEmbed(iKwPack auto&& pack):
		m_color(KwMove<KW_COLOR>(pack)),
		m_title(KwMove<KW_TITLE>(pack)),
		m_description(KwMove<KW_DESCRIPTION>(pack)),
		m_url(KwMove<KW_URL>(pack)),
		m_timestamp(KwMove<KW_TIMESTAMP>(pack)),
		m_thumbnail(KwMove<KW_THUMBNAIL>(pack).Move()),
		m_image(KwMove<KW_IMAGE>(pack).Move()),
		m_footer(KwMove<KW_FOOTER>(pack).Move()),
		m_author(KwMove<KW_AUTHOR>(pack).Move()),
		m_fields(KwMove<KW_FIELDS>(pack)) {}

public:
	cEmbed(iKwArg auto&... kwargs) : cEmbed(cKwPack(kwargs...)) {}

	cEmbed(const json::object&);
	cEmbed(const json::value&);
	cEmbed(const cEmbed& o);
	cEmbed(cEmbed&& o) noexcept = default;

	cEmbed& operator=(cEmbed o);

	/* Non const getters */
	std::string& GetTitle()       noexcept { return m_title;           }
	std::string& GetDescription() noexcept { return m_description;     }
	std::string& GetUrl()         noexcept { return m_url;             }
	std::string& GetTimestamp()   noexcept { return m_timestamp;       }
	hEmbedMedia  GetThumbnail()   noexcept { return m_thumbnail.get(); }
	hEmbedMedia  GetImage()       noexcept { return m_image.get();     }
	hEmbedMedia  GetVideo()       noexcept { return m_video.get();     }
	hEmbedFooter GetFooter()      noexcept { return m_footer.get();    }
	hEmbedAuthor GetAuthor()      noexcept { return m_author.get();    }
	std::vector<cEmbedField>& GetFields() noexcept { return m_fields;  }
	/* Const getters */
	cColor             GetColor()       const noexcept { return m_color;           }
	const std::string& GetTitle()       const noexcept { return m_title;           }
	const std::string& GetDescription() const noexcept { return m_description;     }
	const std::string& GetUrl()         const noexcept { return m_url;             }
	const std::string& GetTimestamp()   const noexcept { return m_timestamp;       }
	chEmbedMedia       GetThumbnail()   const noexcept { return m_thumbnail.get(); }
	chEmbedMedia       GetImage()       const noexcept { return m_image.get();     }
	chEmbedMedia       GetVideo()       const noexcept { return m_video.get();     }
	chEmbedFooter      GetFooter()      const noexcept { return m_footer.get();    }
	chEmbedAuthor      GetAuthor()      const noexcept { return m_author.get();    }
	const std::vector<cEmbedField>& GetFields() const noexcept { return m_fields;  }
	/* Movers */
	std::string   MoveTitle()       noexcept { return std::move(m_title);        }
	std::string   MoveDescription() noexcept { return std::move(m_description);  }
	std::string   MoveUrl()         noexcept { return std::move(m_url);          }
	std::string   MoveTimestamp()   noexcept { return std::move(m_timestamp);    }
	uhEmbedMedia  MoveThumbnail()   noexcept { return std::move(m_thumbnail);    }
	uhEmbedMedia  MoveImage()       noexcept { return std::move(m_image);        }
	uhEmbedMedia  MoveVideo()       noexcept { return std::move(m_video);        }
	uhEmbedFooter MoveFooter()      noexcept { return std::move(m_footer);       }
	uhEmbedAuthor MoveAuthor()      noexcept { return std::move(m_author);       }
	std::vector<cEmbedField> MoveFields() noexcept { return std::move(m_fields); }
	/* Setters */
	cEmbed& SetColor(cColor c) { m_color = c; return *this; }
	template<typename Arg, typename... Args>
	cEmbed& SetTitle(Arg&& arg, Args&&... args) { m_title = { std::forward<Arg>(arg), std::forward<Args>(args)... }; return *this; }
	template<typename Arg, typename... Args>
	cEmbed& SetDescription(Arg&& arg, Args&&... args) { m_description = { std::forward<Arg>(arg), std::forward<Args>(args)... }; return *this; }
	template<typename Arg, typename... Args>
	cEmbed& SetUrl(Arg&& arg, Args&&... args) { m_url = { std::forward<Arg>(arg), std::forward<Args>(args)... }; return *this; }
	template<typename Arg, typename... Args>
	cEmbed& SetTimestamp(Arg&& arg, Args&&... args) { m_timestamp = { std::forward<Arg>(arg), std::forward<Args>(args)...}; return *this; }
	template<typename Arg, typename... Args>
	cEmbed& SetThumbnail(Arg&& arg, Args&&... args) {
		if (m_thumbnail)
			*m_thumbnail = { std::forward<Arg>(arg), std::forward<Args>(args)...};
		else
			m_thumbnail = cHandle::MakeUnique<cEmbedMedia>(std::forward<Arg>(arg), std::forward<Args>(args)...);
		return *this;
	}
	template<typename Arg, typename... Args>
	cEmbed& SetImage(Arg&& arg, Args&&... args) {
		if (m_image)
			*m_image = { std::forward<Arg>(arg), std::forward<Args>(args)...};
		else
			m_image = cHandle::MakeUnique<cEmbedMedia>(std::forward<Arg>(arg), std::forward<Args>(args)...);
		return *this;
	}
	template<typename Arg, typename... Args>
	cEmbed& SetFooter(Arg&& arg, Args&&... args) {
		if (m_footer)
			*m_footer = { std::forward<Arg>(arg), std::forward<Args>(args)...};
		else
			m_footer = cHandle::MakeUnique<cEmbedFooter>(std::forward<Arg>(arg), std::forward<Args>(args)...);
		return *this;
	}
	template<typename Arg, typename... Args>
	cEmbed& SetAuthor(Arg&& arg, Args&&... args) {
		if (m_author)
			*m_author = { std::forward<Arg>(arg), std::forward<Args>(args)... };
		else
			m_author = cHandle::MakeUnique<cEmbedAuthor>(std::forward<Arg>(arg), std::forward<Args>(args)...);
		return *this;
	}
	template<typename Arg, typename... Args>
	cEmbed& SetFields(Arg&& arg, Args&&... args) { m_fields = { std::forward<Arg>(arg), std::forward<Args>(args)... }; return *this; }
	template<typename Arg, typename... Args>
	cEmbed& AddField(Arg&& arg, Args&&... args) { m_fields.emplace_back(std::forward<Arg>(arg), std::forward<Args>(args)...); return *this; }
};
/* Setters for nullptr */
template<>
inline cEmbed& cEmbed::SetTitle<std::nullptr_t>(std::nullptr_t&&) { m_title.clear(); return *this; }
template<>
inline cEmbed& cEmbed::SetDescription<std::nullptr_t>(std::nullptr_t&&) { m_description.clear(); return *this; }
template<>
inline cEmbed& cEmbed::SetUrl<std::nullptr_t>(std::nullptr_t&&) { m_url.clear(); return *this; }
template<>
inline cEmbed& cEmbed::SetTimestamp<std::nullptr_t>(std::nullptr_t&&) { m_timestamp.clear(); return *this; }
template<>
inline cEmbed& cEmbed::SetThumbnail<std::nullptr_t>(std::nullptr_t&&) { m_thumbnail.reset(); return *this; }
template<>
inline cEmbed& cEmbed::SetImage<std::nullptr_t>(std::nullptr_t&&) { m_image.reset(); return *this; }
template<>
inline cEmbed& cEmbed::SetFooter<std::nullptr_t>(std::nullptr_t&&) { m_footer.reset(); return *this; }
template<>
inline cEmbed& cEmbed::SetAuthor<std::nullptr_t>(std::nullptr_t&&) { m_author.reset(); return *this; }
template<>
inline cEmbed& cEmbed::SetFields<std::nullptr_t>(std::nullptr_t&&) { m_fields.clear(); return *this; }
typedef   hHandle<cEmbed>   hEmbed;
typedef  chHandle<cEmbed>  chEmbed;
typedef  uhHandle<cEmbed>  uhEmbed;
typedef uchHandle<cEmbed> uchEmbed;
typedef  shHandle<cEmbed>  shEmbed;
typedef schHandle<cEmbed> schEmbed;

cEmbed tag_invoke(boost::json::value_to_tag<cEmbed>, const boost::json::value&);
void tag_invoke(const json::value_from_tag&, json::value&, const cEmbedField&);
void tag_invoke(const json::value_from_tag&, json::value&, const cEmbed&);

KW_DECLARE(embeds, KW_EMBEDS, std::vector<cEmbed>)
#endif // GREEKBOT_EMBED_H