#include "Embed.h"
#include "json.h"
#include "Utils.h"
/* ================================================================================================================== */
cEmbedAuthor::cEmbedAuthor(const json::value& v):
	m_name(json::value_to<std::string>(v.at("name"))) {
	const json::object& o = v.as_object();
	if (auto p = o.if_contains("url"))
		m_url = json::value_to<std::string>(*p);
	if (auto p = o.if_contains("icon_url"))
		m_icon_url = json::value_to<std::string>(*p);
	if (auto p = o.if_contains("proxy_icon_url"))
		m_proxy_icon_url = json::value_to<std::string>(*p);
}
/* ================================================================================================================== */
cEmbedField::cEmbedField(const json::value& v):
		m_name(json::value_to<std::string>(v.at("name"))),
		m_value(json::value_to<std::string>(v.at("value"))) {
	auto p = v.get_object().if_contains("inline");
	m_inline = p && p->as_bool();
}
/* ================================================================================================================== */
cEmbedFooter::cEmbedFooter(const json::value& v):
		m_text(json::value_to<std::string>(v.at("text"))) {
	const json::object& o = v.as_object();
	if (auto p = o.if_contains("icon_url"))
		m_icon_url = json::value_to<std::string>(*p);
	if (auto p = o.if_contains("proxy_icon_url"))
		m_proxy_icon_url = json::value_to<std::string>(*p);
}
/* ================================================================================================================== */
cEmbedMedia::cEmbedMedia(const json::value& v) {
	const json::object& o = v.as_object();
	if (auto p = o.if_contains("url"))
		m_url = json::value_to<std::string>(*p);
	if (auto p = o.if_contains("proxy_url"))
		m_proxy_url = json::value_to<std::string>(*p);
	if (auto p = o.if_contains("width"))
		m_width = p->to_number<int>();
	if (auto p = o.if_contains("height"))
		m_height = p->to_number<int>();
}
/* ================================================================================================================== */
cEmbed::cEmbed(const json::value& v) {
	auto& o = v.as_object();
	if (auto p = o.if_contains("color"))
		m_color = json::value_to<cColor>(*p);
	if (auto p = o.if_contains("title"))
		m_title = json::value_to<std::string>(*p);
	if (auto p = o.if_contains("description"))
		m_description = json::value_to<std::string>(*p);
	if (auto p = o.if_contains("url"))
		m_url = json::value_to<std::string>(*p);
	if (auto p = o.if_contains("timestamp"))
		m_timestamp = cUtils::ParseISOTimestamp(p->as_string());
	if (auto p = o.if_contains("thumbnail"))
		m_thumbnail.emplace(*p);
	if (auto p = o.if_contains("image"))
		m_image.emplace(*p);
	if (auto p = o.if_contains("video"))
		m_video.emplace(*p);
	if (auto p = o.if_contains("footer"))
		m_footer.emplace(*p);
	if (auto p = o.if_contains("author"))
		m_author.emplace(*p);
	if (auto p = o.if_contains("fields"))
		m_fields = json::value_to<std::vector<cEmbedField>>(*p);
}
/* ================================================================================================================== */
void
tag_invoke(const json::value_from_tag&, json::value& v, const cEmbedAuthor& e) {
	v = {
		{ "name",     e.GetName()    },
		{ "url",      e.GetUrl()     },
		{ "icon_url", e.GetIconUrl() }
	};
}
void
tag_invoke(const json::value_from_tag&, json::value& v, const cEmbedField& e) {
	v = {
		{ "name",   e.GetName()  },
		{ "value",  e.GetValue() },
		{ "inline", e.IsInline() }
	};
}
void
tag_invoke(const json::value_from_tag&, json::value& v, const cEmbedFooter& e) {
	v = {
		{ "text",     e.GetText()    },
		{ "icon_url", e.GetIconUrl() }
	};
}
void
tag_invoke(const json::value_from_tag&, json::value& v, const cEmbedMedia& e) {
	v = {{ "url", e.GetUrl() }};
}
void
tag_invoke(const json::value_from_tag&, json::value& v, const cEmbed& e) {
	json::object& obj = v.emplace_object();
	obj = {
		{ "color",       e.GetColor().ToInt() },
		{ "title",       e.GetTitle()         },
		{ "description", e.GetDescription()   },
		{ "url",         e.GetUrl()           }
	};
	if (auto ts = e.GetTimestamp(); ts != std::chrono::sys_seconds{})
		obj.emplace("timestamp", cUtils::FormatISOTimestamp(ts));

	json::value_from(e.GetFields(), obj["fields"]);

	if (e.GetThumbnail())
		json::value_from(*e.GetThumbnail(), obj["thumbnail"]);
	if (e.GetImage())
		json::value_from(*e.GetImage(), obj["image"]);
	if (e.GetFooter())
		json::value_from(*e.GetFooter(), obj["footer"]);
	if (e.GetAuthor())
		json::value_from(*e.GetAuthor(), obj["author"]);
}
cEmbedField
tag_invoke(json::value_to_tag<cEmbedField>, const json::value& v) {
	return cEmbedField{ v };
}
cEmbed
tag_invoke(json::value_to_tag<cEmbed>, const json::value& v) {
	return cEmbed{ v };
}