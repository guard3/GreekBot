#include "Embed.h"

/* ================================================================================================= */
cEmbedMedia::cEmbedMedia(const json::object &o) {
	const json::value* v;
	if ((v = o.if_contains("url")))
		url = v->as_string().c_str();
	if ((v = o.if_contains("proxy_url")))
		proxy_url = v->as_string().c_str();
	if ((v = o.if_contains("width")))
		width = v->as_int64();
	if ((v = o.if_contains("height")))
		height = v->as_int64();
}

/* ================================================================================================= */
cBaseEmbedGenericObject::cBaseEmbedGenericObject(std::string str, std::string icon_url) : str(std::move(str)), icon_url(std::move(icon_url)) {}

cBaseEmbedGenericObject::cBaseEmbedGenericObject(std::string str, const json::object &o) : str(std::move(str)) {
	if (auto v = o.if_contains("icon_url"))
		icon_url = v->as_string().c_str();
	if (auto v = o.if_contains("proxy_icon_url"))
		proxy_icon_url = v->as_string().c_str();
}

/* ================================================================================================= */
cEmbedAuthor::cEmbedAuthor(const json::object &o) : cBaseEmbedGenericObject(o.at("name").as_string().c_str(), o) {
	if (auto v = o.if_contains("url"))
		url = v->as_string().c_str();
}

/* ================================================================================================= */
cEmbedField::cEmbedField(const json::object &o) : name(o.at("name").as_string().c_str()), value(o.at("value").as_string().c_str()) {
	if (auto v = o.if_contains("inline"))
		inline_ = v && v->as_bool();
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

cBaseEmbed::cBaseEmbed(const cBaseEmbed &o) : color(o.color), title(o.title), description(o.description), url(o.url), timestamp(o.timestamp), Fields(o.Fields) {
	if (o.footer)
		footer = cHandle::MakeUnique<cEmbedFooter>(*o.footer);
	if (o.author)
		author = cHandle::MakeUnique<cEmbedAuthor>(*o.author);
}

cBaseEmbed&
cBaseEmbed::operator=(const cBaseEmbed &o) {
	color       = o.color;
	title       = o.title;
	description = o.description;
	url         = o.url;
	timestamp   = o.timestamp;
	Fields      = o.Fields;

	if (o.footer)
		footer = cHandle::MakeUnique<cEmbedFooter>(*o.footer);
	if (o.author)
		author = cHandle::MakeUnique<cEmbedAuthor>(*o.author);
	return *this;
}

/* ================================================================================================= */
cEmbed::cEmbed(const json::object &o) : cBaseEmbed(o) {
	const json::value* v;
	if ((v = o.if_contains("type"))) {
		const char* s = v->as_string().c_str();
		if (0 == strcmp(s, "rich"))
			type = EMBED_RICH;
		else if (0 == strcmp(s, "image"))
			type = EMBED_IMAGE;
		else if (0 == strcmp(s, "video"))
			type = EMBED_VIDEO;
		else if (0 == strcmp(s, "gifv"))
			type = EMBED_GIFV;
		else if (0 == strcmp(s, "article"))
			type = EMBED_ARTICLE;
		else
			type = EMBED_LINK;
	}
	if ((v = o.if_contains("thumbnail")))
		thumbnail = cHandle::MakeUnique<cEmbedMedia>(*v);
	if ((v = o.if_contains("image")))
		image = cHandle::MakeUnique<cEmbedMedia>(*v);
	if ((v = o.if_contains("video")))
		video = cHandle::MakeUnique<cEmbedMedia>(*v);
}

cEmbed::cEmbed(cEmbedBuilder&& o) : cBaseEmbed(dynamic_cast<cBaseEmbed&&>(o)), type(EMBED_RICH) {
	class make_unique_enabler : public cEmbedMedia {};
	image     = cHandle::MakeUnique<make_unique_enabler>();
	thumbnail = cHandle::MakeUnique<make_unique_enabler>();
	image->url     = std::move(o.image);
	thumbnail->url = std::move(o.thumbnail);
}

cEmbed::cEmbed(const cEmbed &o) : cBaseEmbed(o), type(o.type) {
	if (o.image)
		image = cHandle::MakeUnique<cEmbedMedia>(*o.image);
	if (o.video)
		video = cHandle::MakeUnique<cEmbedMedia>(*o.video);
	if (o.thumbnail)
		thumbnail = cHandle::MakeUnique<cEmbedMedia>(*o.thumbnail);
}

cEmbed& cEmbed::operator=(cEmbed o) {
	cBaseEmbed::operator=(o);
	std::swap(thumbnail, o.thumbnail);
	std::swap(image,     o.image    );
	std::swap(video,     o.video    );
	type = o.type;
	return *this;
}

cEmbedBuilder
cEmbed::CreateBuilder() { return {}; }

json::object
cEmbed::ToJson() const {
	json::object obj {
		{ "title",       title       },
		{ "description", description },
		{ "url",         url         },
		{ "timestamp",   timestamp   }
	};
	if (color.ToInt() >= 0)
		obj["color"] = color.ToInt();
	if (thumbnail)
		obj["thumbnail"] = thumbnail->ToJson();
	if (image)
		obj["image"] = image->ToJson();
	if (footer)
		obj["footer"] = footer->ToJson();
	if (author)
		obj["author"] = author->ToJson();
	{
		json::array a;
		a.reserve(Fields.size());
		for (auto& f : Fields)
			a.push_back(f.ToJson());
		obj["fields"] = std::move(a);
	}
	return obj;
}

/* ================================================================================================= */
cEmbedBuilder&
cEmbedBuilder::SetFooter(cEmbedFooter v) {
	if (footer)
		*footer = std::move(v);
	else
		footer = cHandle::MakeUnique<cEmbedFooter>(std::move(v));
	return *this;
}
cEmbedBuilder&
cEmbedBuilder::SetFooter(const char *text, const char *url) {
	if (footer)
		*footer = cEmbedFooter(text, url);
	else
		footer = cHandle::MakeUnique<cEmbedFooter>(text, url);
	return *this;
}

cEmbedBuilder&
cEmbedBuilder::SetAuthor(cEmbedAuthor v) {
	if (author)
		*author = std::move(v);
	else
		author = cHandle::MakeUnique<cEmbedAuthor>(std::move(v));
	return *this;
}

cEmbedBuilder&
cEmbedBuilder::SetAuthor(const char *name, const char *url, const char *icon_url) {
	if (author)
		*author = cEmbedAuthor(name, url, icon_url);
	else
		author = cHandle::MakeUnique<cEmbedAuthor>(name, url, icon_url);
	return *this;
}

cEmbedBuilder&
cEmbedBuilder::AddField(cEmbedField f) {
	Fields.push_back(std::move(f));
	return *this;
}

cEmbedBuilder&
cEmbedBuilder::AddField(const char *name, const char *value, bool inline_) {
	Fields.emplace_back(name, value, inline_);
	return *this;
}