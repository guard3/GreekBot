#include "Embed.h"
#include "json.h"
#include <zlib.h>

/* ================================================================================================= */
cEmbedMedia::cEmbedMedia(const json::object &o) {
	const json::value* v;
	if ((v = o.if_contains("url"      ))) url       = v->as_string().c_str();
	if ((v = o.if_contains("proxy_url"))) proxy_url = v->as_string().c_str();
	width  = (v = o.if_contains("width" )) ? v->as_int64() : -1;
	height = (v = o.if_contains("height")) ? v->as_int64() : -1;
}

cEmbedMedia::cEmbedMedia(const json::value& v) : cEmbedMedia(v.as_object()) {}

json::object
cEmbedMedia::ToJson() const {
	return {
		{ "url", url }
	};
}

/* ================================================================================================= */
cEmbedAuthor::cEmbedAuthor(const json::object& o) : name(o.at("name").as_string().c_str()) {
	if (auto v = o.if_contains("url"           )) url            = v->as_string().c_str();
	if (auto v = o.if_contains("icon_url"      )) icon_url       = v->as_string().c_str();
	if (auto v = o.if_contains("proxy_icon_url")) proxy_icon_url = v->as_string().c_str();
}

cEmbedAuthor::cEmbedAuthor(const json::value &v) : cEmbedAuthor(v.as_object()) {}

json::object
cEmbedAuthor::ToJson() const {
	return {
		{ "name",     name     },
		{ "url",      url      },
		{ "icon_url", icon_url }
	};
}

/* ================================================================================================= */
cEmbedFooter::cEmbedFooter(const json::object& o) : text(o.at("text").as_string().c_str()) {
	if (auto v = o.if_contains("icon_url"      )) icon_url       = v->as_string().c_str();
	if (auto v = o.if_contains("proxy_icon_url")) proxy_icon_url = v->as_string().c_str();
}

cEmbedFooter::cEmbedFooter(const json::value &v) : cEmbedFooter(v.as_object()) {}

json::object
cEmbedFooter::ToJson() const {
	return {
		{ "text",     text     },
		{ "icon_url", icon_url }
	};
}

/* ================================================================================================= */
cEmbedField::cEmbedField(const json::object &o) : name(o.at("name").as_string().c_str()), value(o.at("value").as_string().c_str()) {
	auto v = o.if_contains("inline");
	inline_ = v && v->as_bool();
}

cEmbedField::cEmbedField(const json::value& v) : cEmbedField(v.as_object()) {}

json::object
cEmbedField::ToJson() const {
	return {
		{ "name",   name    },
		{ "value",  value   },
		{ "inline", inline_ }
	};
}

/* ================================================================================================= */
cBaseEmbed::cBaseEmbed(const json::object &o) {
	const json::value* v;
	if ((v = o.if_contains("color")))
		color = *v;
	if ((v = o.if_contains("title")))
		title = v->as_string().c_str();
	if ((v = o.if_contains("description")))
		description = v->as_string().c_str();
	if ((v = o.if_contains("url")))
		url = v->as_string().c_str();
	if ((v = o.if_contains("timestamp")))
		timestamp = v->as_string().c_str();
	if ((v = o.if_contains("thumbnail")))
		thumbnail = cHandle::MakeUnique<cEmbedMedia>(*v);
	if ((v = o.if_contains("image")))
		image = cHandle::MakeUnique<cEmbedMedia>(*v);
	if ((v = o.if_contains("footer")))
		footer = cHandle::MakeUnique<cEmbedFooter>(*v);
	if ((v = o.if_contains("author")))
		author = cHandle::MakeUnique<cEmbedAuthor>(*v);
	if ((v = o.if_contains("fields"))) {
		auto& a = v->as_array();
		Fields.reserve(a.size());
		for (auto& e : a)
			Fields.emplace_back(e);
	}
}

cBaseEmbed::cBaseEmbed(const json::value &v) : cBaseEmbed(v.as_object()) {}

cBaseEmbed::cBaseEmbed(const cBaseEmbed &o) : color(o.color), title(o.title), description(o.description), url(o.url), timestamp(o.timestamp), Fields(o.Fields) {
	if (o.thumbnail) thumbnail = cHandle::MakeUnique<cEmbedMedia >(*o.thumbnail);
	if (o.image    ) image     = cHandle::MakeUnique<cEmbedMedia >(*o.image    );
	if (o.footer   ) footer    = cHandle::MakeUnique<cEmbedFooter>(*o.footer   );
	if (o.author   ) author    = cHandle::MakeUnique<cEmbedAuthor>(*o.author   );
}

cBaseEmbed&
cBaseEmbed::operator=(const cBaseEmbed &o) {
	color       = o.color;
	title       = o.title;
	description = o.description;
	url         = o.url;
	timestamp   = o.timestamp;
	Fields      = o.Fields;
	thumbnail   = o.thumbnail ? cHandle::MakeUnique<cEmbedMedia >(*o.thumbnail) : uhEmbedMedia();
	image       = o.image     ? cHandle::MakeUnique<cEmbedMedia >(*o.image    ) : uhEmbedMedia();
	footer      = o.footer    ? cHandle::MakeUnique<cEmbedFooter>(*o.footer   ) : uhEmbedFooter();
	author      = o.author    ? cHandle::MakeUnique<cEmbedAuthor>(*o.author   ) : uhEmbedAuthor();
	return *this;
}

/* ================================================================================================= */
cEmbed::cEmbed(const json::object &o) : cBaseEmbed(o) {
	auto v = o.if_contains("type");
	type = v ? (eEmbedType)crc32(0, (const Byte*)v->as_string().c_str(), v->as_string().size()) : EMBED_RICH;
	if ((v = o.if_contains("video"))) video = cHandle::MakeUnique<cEmbedMedia>(*v);
}

cEmbed::cEmbed(const json::value &v) : cEmbed(v.as_object()) {}

cEmbed::cEmbed(const cEmbed &o) : cBaseEmbed(o), type(o.type) {
	if (o.video) video = cHandle::MakeUnique<cEmbedMedia>(*o.video);
}

cEmbed& cEmbed::operator=(cEmbed o) {
	cBaseEmbed::operator=(o);
	std::swap(video, o.video);
	type = o.type;
	return *this;
}

cEmbedBuilder
cEmbed::CreateBuilder() { return {}; }

json::object
cEmbed::ToJson() const {
	json::object obj {
		{ "color",       color.ToInt() },
		{ "title",       title         },
		{ "description", description   },
		{ "url",         url           },
		{ "timestamp",   timestamp     }
	};
	if (thumbnail)
		obj["thumbnail"] = thumbnail->ToJson();
	if (image)
		obj["image"] = image->ToJson();
	if (footer)
		obj["footer"] = footer->ToJson();
	if (author)
		obj["author"] = author->ToJson();
	json::array a;
	a.reserve(Fields.size());
	for (auto& f : Fields)
		a.push_back(f.ToJson());
	obj["fields"] = std::move(a);
	return obj;
}