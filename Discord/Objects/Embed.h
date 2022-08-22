#pragma once
#ifndef _GREEKBOT_EMBED_H_
#define _GREEKBOT_EMBED_H_
#include "Common.h"
#include <vector>

/* The embed type; considered deprecated, see https://discord.com/developers/docs/resources/channel#embed-object-embed-types */
enum eEmbedType : uint32_t {
	EMBED_RICH    = 0xAACD96AA, // generic embed rendered from embed attributes
	EMBED_IMAGE   = 0xC53D045F, // image embed
	EMBED_VIDEO   = 0x7CC7DA2C, // video embed
	EMBED_GIFV    = 0x4A72F821, // animated gif image embed rendered as a video embed
	EMBED_ARTICLE = 0x023A0E66, // article embed
	EMBED_LINK    = 0x36AC99F1  // link embed
};

/* ================================================================================================= */
class cEmbedMedia final {
private:
	std::string url;       // The source url of the image or video
	std::string proxy_url; // A proxied url of the image or video
	int         width;     // Width
	int         height;    // Height

public:
	cEmbedMedia(const json::object&);
	cEmbedMedia(const json::value&);
	cEmbedMedia(std::string url) : url(std::move(url)), width(-1), height(-1) {}

	const std::string& GetUrl()      const { return url;       }
	const std::string& GetProxyUrl() const { return proxy_url; }
	int                GetWidth()    const { return width;     }
	int                GetHeight()   const { return height;    }

	json::object ToJson() const;
};
typedef   hHandle<cEmbedMedia>   hEmbedMedia;
typedef  chHandle<cEmbedMedia>  chEmbedMedia;
typedef  uhHandle<cEmbedMedia>  uhEmbedMedia;
typedef uchHandle<cEmbedMedia> uchEmbedMedia;
typedef  shHandle<cEmbedMedia>  shEmbedMedia;
typedef schHandle<cEmbedMedia> schEmbedMedia;

/* ================================================================================================= */
class cEmbedAuthor final {
private:
	std::string name, url, icon_url, proxy_icon_url;

public:
	cEmbedAuthor(const json::object&);
	cEmbedAuthor(const json::value&);
	cEmbedAuthor(std::string name, std::string url, std::string icon_url) : name(std::move(name)), icon_url(std::move(icon_url)), url(std::move(url)) {}

	const std::string& GetName()         const { return name;           }
	const std::string& GetUrl()          const { return url;            }
	const std::string& GetIconUrl()      const { return icon_url;       }
	const std::string& GetProxyIconUrl() const { return proxy_icon_url; }

	json::object ToJson() const;
};
typedef   hHandle<cEmbedAuthor>   hEmbedAuthor;
typedef  chHandle<cEmbedAuthor>  chEmbedAuthor;
typedef  uhHandle<cEmbedAuthor>  uhEmbedAuthor;
typedef uchHandle<cEmbedAuthor> uchEmbedAuthor;
typedef  shHandle<cEmbedAuthor>  shEmbedAuthor;
typedef schHandle<cEmbedAuthor> schEmbedAuthor;

/* ================================================================================================= */
class cEmbedFooter final {
private:
	std::string text, icon_url, proxy_icon_url;

public:
	cEmbedFooter(const json::object&);
	cEmbedFooter(const json::value& v);
	cEmbedFooter(std::string text, std::string icon_url) : text(std::move(text)), icon_url(std::move(icon_url)) {}

	const std::string& GetText()         const { return text;           }
	const std::string& GetIconUrl()      const { return icon_url;       }
	const std::string& GetProxyIconUrl() const { return proxy_icon_url; }

	json::object ToJson() const;
};
typedef   hHandle<cEmbedFooter>   hEmbedFooter;
typedef  chHandle<cEmbedFooter>  chEmbedFooter;
typedef  uhHandle<cEmbedFooter>  uhEmbedFooter;
typedef uchHandle<cEmbedFooter> uchEmbedFooter;
typedef  shHandle<cEmbedFooter>  shEmbedFooter;
typedef schHandle<cEmbedFooter> schEmbedFooter;

/* ================================================================================================= */
class cEmbedField final {
private:
	std::string name;
	std::string value;
	bool        inline_;

public:
	cEmbedField(const json::object&);
	cEmbedField(const json::value&);
	cEmbedField(std::string name, std::string value, bool inline_ = false) : name(std::move(name)), value(std::move(value)), inline_(inline_) {}

	const std::string& GetName()  const { return name;    }
	const std::string& GetValue() const { return value;   }
	bool               IsInline() const { return inline_; }

	json::object ToJson() const;
};
typedef   hHandle<cEmbedField>   hEmbedField;
typedef  chHandle<cEmbedField>  chEmbedField;
typedef  uhHandle<cEmbedField>  uhEmbedField;
typedef uchHandle<cEmbedField> uchEmbedField;
typedef  shHandle<cEmbedField>  shEmbedField;
typedef schHandle<cEmbedField> schEmbedField;

/* ================================================================================================= */
class cBaseEmbed {
protected:
	cColor        color;
	std::string   title;
	std::string   description;
	std::string   url;
	std::string   timestamp;
	uhEmbedMedia  thumbnail;
	uhEmbedMedia  image;
	uhEmbedFooter footer;
	uhEmbedAuthor author;
	std::vector<cEmbedField> Fields;

	cBaseEmbed() = default;
	cBaseEmbed(const json::object&);
	cBaseEmbed(const json::value&);
	cBaseEmbed(const cBaseEmbed&);
	cBaseEmbed(cBaseEmbed&&) noexcept = default;

	cBaseEmbed& operator=(const cBaseEmbed& o);
	cBaseEmbed& operator=(cBaseEmbed&&) noexcept = default;
};

/* ================================================================================================= */
class cEmbedBuilder;
class cEmbed final : public cBaseEmbed {
	friend class cEmbedBuilder;
private:
	eEmbedType   type;
	uhEmbedMedia video;

	cEmbed(cEmbedBuilder&& o) : cBaseEmbed((cBaseEmbed&&)o), type(EMBED_RICH) {}

public:
	using cBaseEmbed::Fields;

	cEmbed() : type(EMBED_RICH) {}
	cEmbed(const json::object&);
	cEmbed(const json::value&);
	cEmbed(const cEmbed& o);
	cEmbed(cEmbed&& o) noexcept = default;

	cEmbed& operator=(cEmbed o);

	eEmbedType         GetType()        const { return type;            }
	cColor             GetColor()       const { return color;           }
	chEmbedMedia       GetThumbnail()   const { return thumbnail.get(); }
	chEmbedMedia       GetImage()       const { return image.get();     }
	chEmbedMedia       GetVideo()       const { return video.get();     }
	const std::string& GetTitle()       const { return title;           }
	const std::string& GetDescription() const { return description;     }
	const std::string& GetUrl()         const { return url;             }
	const std::string& GetTimestamp()   const { return timestamp;       }

	static cEmbedBuilder CreateBuilder();

	json::object ToJson() const;
};
typedef   hHandle<cEmbed>   hEmbed;
typedef  chHandle<cEmbed>  chEmbed;
typedef  uhHandle<cEmbed>  uhEmbed;
typedef uchHandle<cEmbed> uchEmbed;
typedef  shHandle<cEmbed>  shEmbed;
typedef schHandle<cEmbed> schEmbed;

/* ================================================================================================= */
class cEmbedBuilder final : public cBaseEmbed {
	friend class cEmbed;
private:
	cEmbedBuilder() : cBaseEmbed() {}

public:
	cEmbedBuilder& SetColor(cColor c) {
		color = c;
		return *this;
	}
	cEmbedBuilder& SetDescription(std::string d) {
		description = std::move(d);
		return *this;
	}
	cEmbedBuilder& SetTimestamp(std::string t) {
		timestamp = std::move(t);
		return *this;
	}
	cEmbedBuilder& SetTitle(std::string t, std::string u = {}) {
		title = std::move(t);
		url   = std::move(u);
		return *this;
	}
	cEmbedBuilder& SetThumbnail(cEmbedMedia t) {
		thumbnail = cHandle::MakeUnique<cEmbedMedia>(std::move(t));
		return *this;
	}
	cEmbedBuilder& SetImage(cEmbedMedia t) {
		image = cHandle::MakeUnique<cEmbedMedia>(std::move(t));
		return *this;
	}
	cEmbedBuilder& SetThumbnail(std::string t) {
		thumbnail = cHandle::MakeUnique<cEmbedMedia>(std::move(t));
		return *this;
	}
	cEmbedBuilder& SetImage(std::string i) {
		image = cHandle::MakeUnique<cEmbedMedia>(std::move(i));
		return *this;
	}
	cEmbedBuilder& SetFooter(cEmbedFooter v) {
		footer = cHandle::MakeUnique<cEmbedFooter>(std::move(v));
		return *this;
	}
	cEmbedBuilder& SetFooter(std::string text, std::string url) {
		footer = cHandle::MakeUnique<cEmbedFooter>(std::move(text), std::move(url));
		return *this;
	}
	cEmbedBuilder& SetAuthor(cEmbedAuthor v) {
		author = cHandle::MakeUnique<cEmbedAuthor>(std::move(v));
		return *this;
	}
	cEmbedBuilder& SetAuthor(std::string name, std::string url, std::string icon_url) {
		author = cHandle::MakeUnique<cEmbedAuthor>(std::move(name), std::move(url), std::move(icon_url));
		return *this;
	}
	cEmbedBuilder& AddField(cEmbedField f) {
		Fields.push_back(std::move(f));
		return *this;
	}
	cEmbedBuilder& AddField(std::string name, std::string value, bool inline_ = false) {
		Fields.emplace_back(std::move(name), std::move(value), inline_);
		return *this;
	}

	cEmbed Build() { return std::move(*this); }
};
typedef   hHandle<cEmbedBuilder>   hEmbedBuilder;
typedef  chHandle<cEmbedBuilder>  chEmbedBuilder;
typedef  uhHandle<cEmbedBuilder>  uhEmbedBuilder;
typedef uchHandle<cEmbedBuilder> uchEmbedBuilder;
typedef  shHandle<cEmbedBuilder>  shEmbedBuilder;
typedef schHandle<cEmbedBuilder> schEmbedBuilder;
#endif /* _GREEKBOT_EMBED_H_ */