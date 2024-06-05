#include "Embed.h"
#include "Utils.h"
#include "json.h"
/* ================================================================================================================== */
cEmbedAuthor::cEmbedAuthor(const json::value& v): cEmbedAuthor(v.as_object()) {}
cEmbedAuthor::cEmbedAuthor(const json::object& o):
	m_name(json::value_to<std::string>(o.at("name"))),
	m_url([&o] {
		const auto p = o.if_contains("url");
		return p ? json::value_to<std::string>(*p) : std::string();
	}()),
	m_icon_url([&o] {
		const auto p = o.if_contains("icon_url");
		return p ? json::value_to<std::string>(*p) : std::string();
	}()),
	m_proxy_icon_url([&o] {
		const auto p = o.if_contains("proxy_icon_url");
		return p ? json::value_to<std::string>(*p) : std::string();
	}()) {}
/* ================================================================================================================== */
cEmbedAuthor
tag_invoke(json::value_to_tag<cEmbedAuthor>, const json::value& v) {
	return cEmbedAuthor{ v };
}
void
tag_invoke(json::value_from_tag, json::value& v, const cEmbedAuthor& e) {
	auto& obj = v.emplace_object();
	obj.emplace("name", e.GetName());
	if (auto url = e.GetUrl(); !url.empty())
		obj.emplace("url", url);
	if (auto icon_url = e.GetIconUrl(); !icon_url.empty())
		obj.emplace("icon_url", icon_url);
}
/* ================================================================================================================== */
cEmbedField::cEmbedField(const json::value& v): cEmbedField(v.as_object()) {}
cEmbedField::cEmbedField(const json::object& o):
	m_name(json::value_to<std::string>(o.at("name"))),
	m_value(json::value_to<std::string>(o.at("value"))) {
	const auto p = o.if_contains("inline");
	m_inline = p && p->as_bool();
}
/* ================================================================================================================== */
cEmbedField
tag_invoke(json::value_to_tag<cEmbedField>, const json::value& v) {
	return cEmbedField{ v };
}
void
tag_invoke(json::value_from_tag, json::value& v, const cEmbedField& e) {
	auto& obj = v.emplace_object();
	obj.emplace("name", e.GetName());
	obj.emplace("value", e.GetValue());
	if (e.IsInline())
		obj.emplace("inline", true);
}
/* ================================================================================================================== */
cEmbedFooter::cEmbedFooter(const json::value& v): cEmbedFooter(v.as_object()) {}
cEmbedFooter::cEmbedFooter(const json::object& o):
	m_text(json::value_to<std::string>(o.at("text"))),
	m_icon_url([&o] {
		const auto p = o.if_contains("icon_url");
		return p ? json::value_to<std::string>(*p) : std::string();
	}()),
	m_proxy_icon_url([&o] {
		const auto p = o.if_contains("proxy_icon_url");
		return p ? json::value_to<std::string>(*p) : std::string();
	}()) {}
/* ================================================================================================================== */
cEmbedFooter
tag_invoke(json::value_to_tag<cEmbedFooter>, const json::value& v) {
	return cEmbedFooter{ v };
}
void
tag_invoke(json::value_from_tag, json::value& v, const cEmbedFooter& e) {
	auto& obj = v.emplace_object();
	obj.emplace("text", e.GetText());
	if (auto icon_url = e.GetIconUrl(); !icon_url.empty())
		obj.emplace("icon_url", icon_url);
}
/* ================================================================================================================== */
cEmbedMedia::cEmbedMedia(const json::value& v): cEmbedMedia(v.as_object()) {}
cEmbedMedia::cEmbedMedia(const json::object& o):
	m_url([&o] {
		const auto p = o.if_contains("url");
		return p ? json::value_to<std::string>(*p) : std::string();
	}()),
	m_proxy_url([&o] {
		const auto p = o.if_contains("proxy_url");
		return p ? json::value_to<std::string>(*p) : std::string();
	}()) {
	auto p = o.if_contains("width");
	m_width = p ? p->to_number<int>() : -1;
	p = o.if_contains("height");
	m_height = p ? p->to_number<int>() : -1;
}
/* ================================================================================================================== */
cEmbedMedia
tag_invoke(json::value_to_tag<cEmbedMedia>, const json::value& v) {
	return cEmbedMedia{ v };
}
void
tag_invoke(json::value_from_tag, json::value& v, const cEmbedMedia& e) {
	v = {{ "url", e.GetUrl() }};
}
/* ================================================================================================================== */
cEmbed::cEmbed(const json::value& v) : cEmbed(v.as_object()) {}
cEmbed::cEmbed(const json::object& o) {
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
cEmbed
tag_invoke(json::value_to_tag<cEmbed>, const json::value& v) {
	return cEmbed{ v };
}
void
tag_invoke(json::value_from_tag, json::value& v, const cEmbed& e) {
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