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

cMessage::cMessage(const json::object &o):
	id(json::value_to<cSnowflake>(o.at("id"))),
	channel_id(json::value_to<cSnowflake>(o.at("channel_id"))),
	author(o.at("author")),
	content(json::value_to<std::string>(o.at("content"))),
	timestamp(json::value_to<std::string>(o.at("timestamp"))),
	type(json::value_to<eMessageType>(o.at("type"))),
	embeds(json::value_to<std::vector<cEmbed>>(o.at("embeds"))) {
	/* Parse guild_id */
	const json::value* f;
	if ((f = o.if_contains("guild_id")))
		guild_id = cHandle::MakeUnique<cSnowflake>(json::value_to<cSnowflake>(*f));
	/* Parse edited_timestamp */
	if (auto s = o.at("edited_timestamp").if_string())
		edited_timestamp = s->c_str();
	/* Parse flags */
	flags = (eMessageFlag)((f = o.if_contains("flags")) ? f->as_int64() : 0);
	flags = flags | (eMessageFlag)((o.at("tts").as_bool() << 15) | (o.at("mention_everyone").as_bool() << 16));
}

cMessage::cMessage(const json::value &v) : cMessage(v.as_object()) {}

cMessage::cMessage(const cMessage &o) : id(o.id), channel_id(o.channel_id), author(o.author), content(o.content), timestamp(o.timestamp), edited_timestamp(o.edited_timestamp), type(o.type), flags(o.flags), embeds(o.embeds) {
	if (o.guild_id) guild_id = cHandle::MakeUnique<cSnowflake>(*o.guild_id);
	if (o.member  ) member   = cHandle::MakeUnique<cMember   >(*o.member  );
}

cMessage&
cMessage::operator=(cMessage o) {
	std::swap(guild_id,         o.guild_id        );
	std::swap(author,           o.author          );
	std::swap(member,           o.member          );
	std::swap(content,          o.content         );
	std::swap(timestamp,        o.timestamp       );
	std::swap(edited_timestamp, o.edited_timestamp);
	id         = o.id;
	channel_id = o.channel_id;
	type       = o.type;
	flags      = o.flags;
	return *this;
}