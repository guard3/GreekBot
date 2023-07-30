#include "Embed.h"
#include "json.h"

/* ================================================================================================= */
cEmbedField::cEmbedField(const json::object &o) : m_name(o.at("name").as_string().c_str()), m_value(o.at("value").as_string().c_str()) {
	auto v = o.if_contains("inline");
	m_inline = v && v->as_bool();
}

cEmbedField::cEmbedField(const json::value& v) : cEmbedField(v.as_object()) {}

json::object
cEmbedField::ToJson() const {
	return {
		{ "name",   m_name   },
		{ "value",  m_value  },
		{ "inline", m_inline }
	};
}
/* ================================================================================================= */
cEmbed::cEmbed(const json::object &o) {
	const json::value* v;
	if ((v = o.if_contains("color")))
		m_color = json::value_to<cColor>(*v);
	if ((v = o.if_contains("title")))
		m_title = json::value_to<std::string>(*v);
	if ((v = o.if_contains("description")))
		m_description = json::value_to<std::string>(*v);
	if ((v = o.if_contains("url")))
		m_url = json::value_to<std::string>(*v);
	if ((v = o.if_contains("timestamp")))
		m_timestamp = json::value_to<std::string>(*v);
	if ((v = o.if_contains("thumbnail")))
		m_thumbnail = cHandle::MakeUnique<cEmbedMedia>(*v);
	if ((v = o.if_contains("image")))
		m_image = cHandle::MakeUnique<cEmbedMedia>(*v);
	if ((v = o.if_contains("video")))
		m_video = cHandle::MakeUnique<cEmbedMedia>(*v);
	if ((v = o.if_contains("footer")))
		m_footer = cHandle::MakeUnique<cEmbedFooter>(*v);
	if ((v = o.if_contains("author")))
		m_author = cHandle::MakeUnique<cEmbedAuthor>(*v);
	if ((v = o.if_contains("fields"))) {
		auto& a = v->as_array();
		m_fields.reserve(a.size());
		for (auto& e : a)
			m_fields.emplace_back(e);
	}
}

cEmbed::cEmbed(const json::value &v) : cEmbed(v.as_object()) {}

cEmbed::cEmbed(const cEmbed &o):
	m_color(o.m_color),
	m_title(o.m_title),
	m_description(o.m_description),
	m_url(o.m_url),
	m_timestamp(o.m_timestamp),
	m_fields(o.m_fields) {
	if (o.m_thumbnail) m_thumbnail = cHandle::MakeUnique<cEmbedMedia >(*o.m_thumbnail);
	if (o.m_image    ) m_image     = cHandle::MakeUnique<cEmbedMedia >(*o.m_image    );
	if (o.m_video    ) m_video     = cHandle::MakeUnique<cEmbedMedia >(*o.m_video    );
	if (o.m_footer   ) m_footer    = cHandle::MakeUnique<cEmbedFooter>(*o.m_footer   );
	if (o.m_author   ) m_author    = cHandle::MakeUnique<cEmbedAuthor>(*o.m_author   );
}

cEmbed&
cEmbed::operator=(cEmbed o) {
	m_color = o.m_color;
	m_title.swap(o.m_title);
	m_description.swap(o.m_description);
	m_url.swap(o.m_url);
	m_timestamp.swap(o.m_timestamp);
	m_thumbnail.swap(o.m_thumbnail);
	m_image.swap(o.m_image);
	m_video.swap(o.m_video);
	m_footer.swap(o.m_footer);
	m_author.swap(o.m_author);
	m_fields.swap(o.m_fields);
	return *this;
}

/* ================================================================================================= */
json::object
cEmbed::ToJson() const {
	json::object obj {
		{ "color",       m_color.ToInt() },
		{ "title",       m_title         },
		{ "description", m_description   },
		{ "url",         m_url           },
		{ "timestamp",   m_timestamp     }
	};
	if (m_thumbnail) obj["thumbnail"] = m_thumbnail->ToJson();
	if (m_image    ) obj["image"    ] = m_image->ToJson();
	if (m_footer   ) obj["footer"   ] = m_footer->ToJson();
	if (m_author   ) obj["author"   ] = m_author->ToJson();
	json::array a;
	a.reserve(m_fields.size());
	for (auto& f : m_fields)
		a.push_back(f.ToJson());
	obj["fields"] = std::move(a);
	return obj;
}