#include "Message.h"
#include "json.h"
/* ================================================================================================================== */
eMessageType
tag_invoke(json::value_to_tag<eMessageType>, const json::value& v) {
	return static_cast<eMessageType>(v.to_number<int>());
}
/* ================================================================================================================== */
void
tag_invoke(json::value_from_tag, json::value& v, const cMessageParams& m) {
	json::object& obj = v.emplace_object();
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
cMessageUpdate::cMessageUpdate(const json::value& v) : cMessageUpdate(v.as_object()) {}
cMessageUpdate::cMessageUpdate(const json::object& o) :
	m_id(json::value_to<std::string_view>(o.at("id"))),
	m_channel_id(json::value_to<cSnowflake>(o.at("channel_id"))) {
	if (auto p = o.if_contains("content"))
		m_content.emplace(json::value_to<std::string>(*p));
	if (auto p = o.if_contains("components"))
		m_components.emplace(json::value_to<std::vector<cActionRow>>(*p));
	// TODO: parse embeds
}
/* ================================================================================================================== */
cMessageUpdate
tag_invoke(json::value_to_tag<cMessageUpdate>, const json::value& v) {
	return cMessageUpdate{ v };
}
void
tag_invoke(json::value_from_tag, json::value& v, const cMessageUpdate& m) {
	json::object& obj = v.emplace_object();
	if (auto p = m.GetContent().Get())
		json::value_from(*p, obj["content"]);
	if (auto p = m.GetComponents().Get())
		json::value_from(*p, obj["components"]);
	if (auto p = m.GetEmbeds().Get())
		json::value_from(*p, obj["embeds"]);
}
/* ================================================================================================================== */
cMessage::cMessage(const json::value &v) : cMessage(v.as_object()) {}
cMessage::cMessage(const json::object &o):
	id(json::value_to<cSnowflake>(o.at("id"))),
	channel_id(json::value_to<cSnowflake>(o.at("channel_id"))),
	author(o.at("author")),
	content(json::value_to<std::string>(o.at("content"))),
	timestamp(json::value_to<std::string>(o.at("timestamp"))),
	type(json::value_to<eMessageType>(o.at("type"))),
	embeds(json::value_to<std::vector<cEmbed>>(o.at("embeds"))),
	attachments(json::value_to<std::vector<cAttachment>>(o.at("attachments"))) {
	/* Parse edited_timestamp */
	if (auto res = json::try_value_to<std::string>(o.at("edited_timestamp")); res.has_value())
		edited_timestamp = std::move(*res);
	/* Parse flags */
	const json::value* f = o.if_contains("flags");
	flags = f ? (eMessageFlag)f->to_number<int>() : MESSAGE_FLAG_NONE;
	flags = flags | (eMessageFlag)((o.at("tts").as_bool() << 15) | (o.at("mention_everyone").as_bool() << 16));
}