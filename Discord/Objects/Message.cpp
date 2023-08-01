#include "Message.h"
#include "json.h"

eMessageType
tag_invoke(json::value_to_tag<eMessageType>, const json::value& v) {
	return static_cast<eMessageType>(v.to_number<int>());
}

json::object
cMessageParams::ToJson() const {
	/* Create object with flags */
	json::object obj {
		{ "tts", (bool)(m_flags & MESSAGE_FLAG_TTS) },
		{ "flags", (int)(m_flags & (MESSAGE_FLAG_EPHEMERAL | MESSAGE_FLAG_SUPPRESS_EMBEDS)) }
	};
	/* Set content */
	if (!m_content)
		obj["content"] = "";
	else if (!m_content->empty())
		obj["content"] = *m_content;
	/* Set components */
	if (!m_components)
		obj["components"] = json::array();
	else if (!m_components->empty()) {
		json::array a;
		a.reserve(m_components->size());
		for (auto& c : *m_components)
			a.push_back(c.ToJson());
		obj["components"] = std::move(a);
	}
	/* Set embeds */
	if (!m_embeds)
		obj["embeds"] = json::array();
	else if (!m_embeds->empty()) {
		json::array a;
		a.reserve(m_embeds->size());
		for (auto& e : *m_embeds)
			a.push_back(e.ToJson());
		obj["embeds"] = std::move(a);
	}
	return obj;
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