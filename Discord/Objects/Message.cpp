#include "Message.h"
#include "Utils.h"
#include <boost/json.hpp>
/* ================================================================================================================== */
namespace json = boost::json;
/* ================================================================================================================== */
eMessageType
tag_invoke(json::value_to_tag<eMessageType>, const json::value& v) {
	return static_cast<eMessageType>(v.to_number<std::underlying_type_t<eMessageType>>());
}
/* ================================================================================================================== */
cMessageUpdate::cMessageUpdate(const json::value& v) : cMessageUpdate(v.as_object()) {}
cMessageUpdate::cMessageUpdate(const json::object& o) :
	m_id(o.at("id").as_string()),
	m_channel_id(o.at("channel_id").as_string()) {
	if (auto p = o.if_contains("content"))
		m_content.emplace(json::value_to<std::string>(*p));
	if (auto p = o.if_contains("components"))
		m_components.emplace(json::value_to<std::vector<cActionRow>>(*p));
	if (auto p = o.if_contains("embeds"))
		m_embeds.emplace(json::value_to<std::vector<cEmbed>>(*p));
}
/* ================================================================================================================== */
cMessageUpdate
tag_invoke(json::value_to_tag<cMessageUpdate>, const json::value& v) {
	return cMessageUpdate{ v };
}
void
tag_invoke(json::value_from_tag, json::value& v, const cMessageUpdate& m) {
	auto& obj = v.emplace_object();
	obj.reserve(3);
	if (auto p = m.GetContent().Get())
		obj.emplace("content", *p);
	if (auto p = m.GetComponents().Get())
		json::value_from(*p, obj["components"]);
	if (auto p = m.GetEmbeds().Get())
		json::value_from(*p, obj["embeds"]);
}
/* ================================================================================================================== */
cMessageBase::cMessageBase(const json::value& v) : cMessageBase(v.as_object()) {}
cMessageBase::cMessageBase(const json::object& o):
	m_flags([&o] {
		const auto p = o.if_contains("flags");
		const auto f = p ? p->to_number<std::underlying_type_t<eMessageFlag>>() : 0;
		return static_cast<eMessageFlag>(f | (o.at("tts").as_bool() << 15) | (o.at("mention_everyone").as_bool() << 16));
	}()),
	m_content(json::value_to<std::string>(o.at("content"))),
	m_components([&o] {
		const auto p = o.if_contains("components");
		return p ? json::value_to<std::vector<cActionRow>>(*p) : std::vector<cActionRow>();
	}()),
	m_embeds(json::value_to<std::vector<cEmbed>>(o.at("embeds"))) {}
/* ================================================================================================================== */
void
tag_invoke(json::value_from_tag, json::value& v, const cMessageBase& m) {
	auto& obj = v.emplace_object();
	obj.reserve(5);
	/* Set flags */
	if (m.GetFlags() & MESSAGE_FLAG_TTS)
		obj.emplace("tts", true);
	if (auto flags = m.GetFlags() & (MESSAGE_FLAG_EPHEMERAL | MESSAGE_FLAG_SUPPRESS_EMBEDS))
		obj.emplace("flags", flags);
	/* Set content */
	if (auto s = m.GetContent(); !s.empty())
		obj.emplace("content", s);
	/* Set components */
	if (auto c = m.GetComponents(); !c.empty())
		json::value_from(c, obj["components"]);
	/* Set embeds */
	if (auto e = m.GetEmbeds(); !e.empty())
		json::value_from(e, obj["embeds"]);
}
/* ================================================================================================================== */
cMessage::cMessage(const json::value&  v) : cMessage(v.as_object()) {}
cMessage::cMessage(const json::object& o) :
	cMessageBase(o),
	id(o.at("id").as_string()),
	channel_id(o.at("channel_id").as_string()),
	author(o.at("author")),
	timestamp(cUtils::ParseISOTimestamp(o.at("timestamp").as_string())),
	type(json::value_to<eMessageType>(o.at("type"))),
	attachments(json::value_to<std::vector<cAttachment>>(o.at("attachments"))) {
	/* Parse edited_timestamp */
	if (auto& v = o.at("edited_timestamp"); !v.is_null())
		edited_timestamp = cUtils::ParseISOTimestamp(v.as_string());
}
/* ================================================================================================================== */
cMessage
tag_invoke(json::value_to_tag<cMessage>, const json::value& v) {
	return cMessage{ v };
}