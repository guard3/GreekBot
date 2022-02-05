#include "Embed.h"

/* ================================================================================================= */
cBaseEmbed::cBaseEmbed() : color(-1), footer(nullptr), author(nullptr) {}

cBaseEmbed::cBaseEmbed(const cBaseEmbed &o) : color(o.color), title(o.title), description(o.description), url(o.url), timestamp(o.timestamp), fields(o.fields) {
	uhEmbedFooter u_footer = o.footer ? cHandle::MakeUnique<cEmbedFooter>(*o.footer) : uhEmbedFooter();
	author = o.author ? new cEmbedAuthor(*o.author) : nullptr;
	footer = u_footer.release();
}

cBaseEmbed::cBaseEmbed(cBaseEmbed &&o) noexcept : color(o.color), title(std::move(o.title)), description(std::move(o.description)), url(std::move(o.url)), timestamp(std::move(o.timestamp)), footer(o.footer), author(o.author), fields(std::move(o.fields)) {
	o.footer = nullptr;
	o.author = nullptr;
	o.color  = -1;
}

cBaseEmbed::~cBaseEmbed() {
	delete footer;
	delete author;
}

cBaseEmbed&
cBaseEmbed::operator=(const cBaseEmbed &o) {
	color       = o.color;
	title       = o.title;
	description = o.description;
	url         = o.url;
	timestamp   = o.timestamp;
	fields      = o.fields;

	uhEmbedFooter u_footer = o.footer ? cHandle::MakeUnique<cEmbedFooter>(*o.footer) : uhEmbedFooter();
	author = o.author ? new cEmbedAuthor(*o.author) : nullptr;
	footer = u_footer.release();
	return *this;
}

cBaseEmbed&
cBaseEmbed::operator=(cBaseEmbed&& o) noexcept {
	std::swap(color,       o.color      );
	std::swap(title,       o.title      );
	std::swap(description, o.description);
	std::swap(url,         o.url        );
	std::swap(timestamp,   o.timestamp  );
	std::swap(footer,      o.footer     );
	std::swap(author,      o.author     );
	std::swap(fields,      o.fields     );
	return *this;
}

/* ================================================================================================= */
cEmbed::cEmbed(cEmbedBuilder&& o) : cBaseEmbed(dynamic_cast<cBaseEmbed&&>(o)), type(EMBED_RICH), thumbnail(nullptr), video(nullptr) {
	image = new cEmbedMedia();
	try {
		thumbnail = new cEmbedMedia();
	}
	catch (std::exception& e) {
		delete image;
		throw(e);
	}
	image->url     = std::move(o.image);
	thumbnail->url = std::move(o.thumbnail);
}

cEmbed::cEmbed(const cEmbed &o) : cBaseEmbed(o), type(o.type) {
	uhEmbedMedia u_image = o.image ? cHandle::MakeUnique<cEmbedMedia>(*o.image) : uhEmbedMedia();
	uhEmbedMedia u_video = o.video ? cHandle::MakeUnique<cEmbedMedia>(*o.video) : uhEmbedMedia();
	thumbnail = o.thumbnail ? new cEmbedMedia(*o.thumbnail) : nullptr;
	image = u_image.release();
	video = u_video.release();
}

cEmbed::cEmbed(cEmbed&& o) noexcept : cBaseEmbed(std::forward<cEmbed>(o)), type(o.type), thumbnail(o.thumbnail), image(o.image), video(o.video) {
	o.type = EMBED_RICH;
	o.thumbnail = o.image = o.video = nullptr;
}

cEmbed::~cEmbed() {
	delete thumbnail;
	delete image;
	delete video;
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
	if (color >= 0)
		obj["color"] = color;
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
		a.reserve(fields.size());
		for (auto& f : fields)
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
		footer = new cEmbedFooter(std::move(v));
	return *this;
}
cEmbedBuilder&
cEmbedBuilder::SetFooter(const char *text, const char *url) {
	if (footer)
		*footer = cEmbedFooter(text, url);
	else
		footer = new cEmbedFooter(text, url);
	return *this;
}

cEmbedBuilder&
cEmbedBuilder::SetAuthor(cEmbedAuthor v) {
	if (author)
		*author = std::move(v);
	else
		author = new cEmbedAuthor(std::move(v));
	return *this;
}

cEmbedBuilder&
cEmbedBuilder::SetAuthor(const char *name, const char *url, const char *icon_url) {
	if (author)
		*author = cEmbedAuthor(name, url, icon_url);
	else
		author = new cEmbedAuthor(name, url, icon_url);
	return *this;
}

cEmbedBuilder&
cEmbedBuilder::AddField(cEmbedField f) {
	fields.push_back(std::move(f));
	return *this;
}

cEmbedBuilder&
cEmbedBuilder::AddField(const char *name, const char *value, bool inline_) {
	fields.emplace_back(name, value, inline_);
	return *this;
}