#include "Message.h"

cMessage::cMessage(const json::object &o) : id(o.at("id")), channel_id(o.at("channel_id")), author(o.at("author")), content(o.at("content").as_string().c_str()), timestamp(o.at("timestamp").as_string().c_str()), type((eMessageType)o.at("type").as_int64()) {
	/* Parse guild_id */
	const json::value* f;
	if ((f = o.if_contains("guild_id")))
		guild_id = cHandle::MakeUnique<cSnowflake>(*f);
	/* Parse edited_timestamp */
	if (auto s = o.at("edited_timestamp").if_string())
		edited_timestamp = s->c_str();
	/* Parse flags */
	flags = (eMessageFlag)((f = o.if_contains("flags")) ? f->as_int64() : 0);
	flags = flags | (eMessageFlag)((o.at("tts").as_bool() << 15) | (o.at("mention_everyone").as_bool() << 16));
	/* Parse embeds */
	auto& a = o.at("embeds").as_array();
	Embeds.reserve(a.size());
	for (auto& v : a)
		Embeds.emplace_back(v);
}

cMessage::cMessage(const cMessage &o) : id(o.id), channel_id(o.channel_id), author(o.author), content(o.content), timestamp(o.timestamp), edited_timestamp(o.edited_timestamp), type(o.type), flags(o.flags), Embeds(o.Embeds) {
	if (o.guild_id)
		guild_id = cHandle::MakeUnique<cSnowflake>(*o.guild_id);
	if (o.member)
		member = cHandle::MakeUnique<cMember>(*o.member);
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
