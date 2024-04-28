#include "Message.h"
#include "json.h"

eMessageType
tag_invoke(json::value_to_tag<eMessageType>, const json::value& v) {
	return static_cast<eMessageType>(v.to_number<int>());
}

void
tag_invoke(const json::value_from_tag&, json::value& v, const cMessageParams& m) {
	json::object& obj = v.emplace_object();
	/* Set flags */
	obj = {
		{ "tts", (bool)(m.m_flags & MESSAGE_FLAG_TTS) },
		{ "flags", m.m_flags & (MESSAGE_FLAG_EPHEMERAL | MESSAGE_FLAG_SUPPRESS_EMBEDS) }
	};
	/* Set content */
	if (!m.m_content)
		obj["content"].emplace_string();
	else if (!m.m_content->empty())
		obj.emplace("content", *m.m_content);
	/* Set components */
	if (!m.m_components)
		obj["components"].emplace_array();
	else if (!m.m_components->empty())
		json::value_from(*m.m_components, obj["components"]);
	/* Set embeds */
	if (!m.m_embeds)
		obj["embeds"].emplace_array();
	else if (!m.m_embeds->empty())
		json::value_from(*m.m_embeds, obj["embeds"]);
}

cMessageUpdate::cMessageUpdate(const json::object& o) :
	m_id(json::value_to<std::string_view>(o.at("id"))),
	m_channel_id(json::value_to<cSnowflake>(o.at("channel_id"))) {
	if (auto p = o.if_contains("content"))
		m_content.emplace(json::value_to<std::string>(*p));
}
cMessageUpdate::cMessageUpdate(const json::value& v) : cMessageUpdate(v.as_object()) {}

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

cMessage::cMessage(const json::value &v) : cMessage(v.as_object()) {}