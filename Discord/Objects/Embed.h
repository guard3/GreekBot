#pragma once
#ifndef _GREEKBOT_EMBED_H_
#define _GREEKBOT_EMBED_H_
#include "Types.h"
#include <vector>

/* The embed type; considered deprecated, see https://discord.com/developers/docs/resources/channel#embed-object-embed-types */
enum eEmbedType {
	EMBED_RICH,    // generic embed rendered from embed attributes
	EMBED_IMAGE,   // image embed
	EMBED_VIDEO,   // video embed
	EMBED_GIFV,    // animated gif image embed rendered as a video embed
	EMBED_ARTICLE, // article embed
	EMBED_LINK     // link embed
};

/* ================================================================================================= */
class cEmbedMedia {
	friend class cEmbed;
private:
	std::string url;       // The source url of the image or video
	std::string proxy_url; // A proxied url of the image or video
	int         width;     // Width
	int         height;    // Height

	cEmbedMedia() : width(-1), height(-1) {}

public:
	cEmbedMedia(const json::object& o);
	cEmbedMedia(const json::value& v) : cEmbedMedia(v.as_object()) {}

	const char* GetUrl()      const { return       url.empty() ? nullptr :       url.c_str(); }
	const char* GetProxyUrl() const { return proxy_url.empty() ? nullptr : proxy_url.c_str(); }
	int         GetWidth()    const { return width;  }
	int         GetHeight()   const { return height; }

	json::object ToJson() const { return { { "url", url } }; }
};
typedef   hHandle<cEmbedMedia>   hEmbedMedia;
typedef  chHandle<cEmbedMedia>  chEmbedMedia;
typedef  uhHandle<cEmbedMedia>  uhEmbedMedia;
typedef uchHandle<cEmbedMedia> uchEmbedMedia;
typedef  shHandle<cEmbedMedia>  shEmbedMedia;
typedef schHandle<cEmbedMedia> schEmbedMedia;

/* ================================================================================================= */
class cBaseEmbedGenericObject {
private:
	std::string str;            // A generic string
	std::string icon_url;       // Url of the icon
	std::string proxy_icon_url; // A proxied url of the icon

protected:
	cBaseEmbedGenericObject(std::string str, std::string icon_url);
	cBaseEmbedGenericObject(std::string str, const json::object& o);
	cBaseEmbedGenericObject(std::string str, const json::value& v) : cBaseEmbedGenericObject(std::move(str), v.as_object()) {}

	const std::string& GetString() const { return str; }

public:
	const char* GetIconUrl()      const { return       icon_url.empty() ? nullptr :       icon_url.c_str(); }
	const char* GetProxyIconUrl() const { return proxy_icon_url.empty() ? nullptr : proxy_icon_url.c_str(); }

	virtual json::object ToJson() const { return { { "icon_url", icon_url } }; }
};

/* ================================================================================================= */
class cEmbedAuthor final : public cBaseEmbedGenericObject {
private:
	std::string url;

public:
	cEmbedAuthor(const json::object& o);
	cEmbedAuthor(const json::value& v) : cEmbedAuthor(v.as_object()) {}
	cEmbedAuthor(const char* name, const char* url, const char* icon_url) : cBaseEmbedGenericObject(name, icon_url ? icon_url : std::string()), url(url ? url : std::string()) {}

	const char* GetName() const { return GetString().c_str();                 }
	const char* GetUrl()  const { return url.empty() ? nullptr : url.c_str(); }

	json::object ToJson() const override {
		json::object obj = cBaseEmbedGenericObject::ToJson();
		obj["name"] = GetString();
		obj["url"] = url;
		return obj;
	}
};
typedef   hHandle<cEmbedAuthor>   hEmbedAuthor;
typedef  chHandle<cEmbedAuthor>  chEmbedAuthor;
typedef  uhHandle<cEmbedAuthor>  uhEmbedAuthor;
typedef uchHandle<cEmbedAuthor> uchEmbedAuthor;
typedef  shHandle<cEmbedAuthor>  shEmbedAuthor;
typedef schHandle<cEmbedAuthor> schEmbedAuthor;

/* ================================================================================================= */
class cEmbedFooter final : public cBaseEmbedGenericObject {
public:
	cEmbedFooter(const json::value& v) : cEmbedFooter(v.as_object()) {}
	cEmbedFooter(const json::object& o) : cBaseEmbedGenericObject(o.at("text").as_string().c_str(), o) {}
	cEmbedFooter(const char* text, const char* icon_url) : cBaseEmbedGenericObject(text, icon_url ? icon_url : std::string()) {}

	const char* GetText() const { return GetString().c_str(); }

	json::object ToJson() const override {
		json::object obj = cBaseEmbedGenericObject::ToJson();
		obj["text"] = GetString();
		return obj;
	}
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
	cEmbedField(const json::value& v) : cEmbedField(v.as_object()) {}
	cEmbedField(const json::object& o);
	cEmbedField(const char* name, const char* value, bool inline_ = false) : name(name), value(value), inline_(inline_) {}

	const char* GetName()  const { return name.c_str();  }
	const char* GetValue() const { return value.c_str(); }
	bool        IsInline() const { return inline_;       }

	json::object ToJson() const {
		return {
			{ "name",   name    },
			{ "value",  value   },
			{ "inline", inline_ }
		};
	}
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
	uhEmbedFooter footer;
	uhEmbedAuthor author;
	std::vector<cEmbedField> Fields;

	cBaseEmbed() = default;
	cBaseEmbed(const json::object&);
	cBaseEmbed(const json::value& v) : cBaseEmbed(v.as_object()) {}
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
	uhEmbedMedia thumbnail;
	uhEmbedMedia image;
	uhEmbedMedia video;

	cEmbed(cEmbedBuilder&& o);

public:
	using cBaseEmbed::Fields;

	cEmbed() : type(EMBED_RICH) {}
	cEmbed(const json::object&);
	cEmbed(const json::value& v) : cEmbed(v.as_object()) {}
	cEmbed(const cEmbed& o);
	cEmbed(cEmbed&& o) noexcept = default;

	cEmbed& operator=(cEmbed o);

	eEmbedType   GetType()      const { return type;            }
	cColor       GetColor()     const { return color;           }
	chEmbedMedia GetThumbnail() const { return thumbnail.get(); }
	chEmbedMedia GetImage()     const { return image.get();     }
	chEmbedMedia GetVideo()     const { return video.get();     }
	const char*  GetTitle()       const { return       title.empty() ? nullptr :       title.c_str(); }
	const char*  GetDescription() const { return description.empty() ? nullptr : description.c_str(); }
	const char*  GetUrl()         const { return         url.empty() ? nullptr :         url.c_str(); }
	const char*  GetTimestamp()   const { return   timestamp.empty() ? nullptr :   timestamp.c_str(); }

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
	std::string thumbnail;
	std::string image;

	cEmbedBuilder() : cBaseEmbed() {}

public:
	cEmbedBuilder& SetColor(int c)               { color       = c; return *this; }
	cEmbedBuilder& SetDescription(const char* d) { description = d; return *this; }
	cEmbedBuilder& SetTimestamp(const char* t)   { timestamp   = t; return *this; }
	cEmbedBuilder& SetThumbnail(const char* t)   { thumbnail   = t; return *this; }
	cEmbedBuilder& SetImage(const char* i)       { image       = i; return *this; }
	cEmbedBuilder& SetTitle(const char* t)       { title       = t; return *this; }
	cEmbedBuilder& SetTitle(const char* t, const char* u) { url = u; return SetTitle(t); }

	cEmbedBuilder& SetFooter(cEmbedFooter v);
	cEmbedBuilder& SetFooter(const char* text, const char* url);
	cEmbedBuilder& SetAuthor(cEmbedAuthor v);
	cEmbedBuilder& SetAuthor(const char* name, const char* url, const char* icon_url);
	cEmbedBuilder& AddField(cEmbedField f);
	cEmbedBuilder& AddField(const char* name, const char* value, bool inline_ = false);

	cEmbed Build() { return std::move(*this); }
};
typedef   hHandle<cEmbedBuilder>   hEmbedBuilder;
typedef  chHandle<cEmbedBuilder>  chEmbedBuilder;
typedef  uhHandle<cEmbedBuilder>  uhEmbedBuilder;
typedef uchHandle<cEmbedBuilder> uchEmbedBuilder;
typedef  shHandle<cEmbedBuilder>  shEmbedBuilder;
typedef schHandle<cEmbedBuilder> schEmbedBuilder;
#endif /* _GREEKBOT_EMBED_H_ */