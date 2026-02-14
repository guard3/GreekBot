#include "Component/ComponentType.h"
#include "Message.h"
#include "Utils.h"
#include <boost/json.hpp>

namespace json = boost::json;

static auto get_message_flags(const json::object& o) {
	auto flags = static_cast<eMessageFlag>(o.at("tts").as_bool() << 30u | o.at("mention_everyone").as_bool() << 31u);
	if (auto p = o.if_contains("flags"))
		flags = flags | json::value_to<eMessageFlag>(*p);

	return flags;
}

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

cPartialMessage::cPartialMessage(const json::value& v) : cPartialMessage(v.as_object()) {}
cPartialMessage::cPartialMessage(const json::object& o): cPartialMessage(get_message_flags(o), o) {}
cPartialMessage::cPartialMessage(eMessageFlag flags, const json::object& o) :
	cMessageBase(flags),
	m_content(json::value_to<std::string>(o.at("content"))),
	m_components([&o] {
		const auto p = o.if_contains("components");
		return p ? json::value_to<std::vector<cActionRow>>(*p) : std::vector<cActionRow>();
	}()),
	m_embeds(json::value_to<std::vector<cEmbed>>(o.at("embeds"))) {}

cPartialMessageV2::cPartialMessageV2(const json::value& v) : cPartialMessageV2(v.as_object()) {}
cPartialMessageV2::cPartialMessageV2(const json::object& o): cPartialMessageV2(get_message_flags(o), o) {}
cPartialMessageV2::cPartialMessageV2(eMessageFlag flags, const json::object& o) :
	cMessageBase(flags),
	m_components(json::value_to<std::vector<component_type>>(o.at("components"))) {}

cMessage::cMessage(const json::value&  v) : cMessage(v.as_object()) {}
cMessage::cMessage(const json::object& o) :
	m_data([&] {
		using variant_type = std::variant<cPartialMessage, cPartialMessageV2>;
		auto flags = get_message_flags(o);
		return flags & MESSAGE_FLAG_IS_COMPONENTS_V2
		             ? variant_type(std::in_place_type<cPartialMessageV2>, flags, o)
		             : variant_type(std::in_place_type<cPartialMessage>, flags, o);
	}()),
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

/** @name JSON value to object conversion
 */
/// @{
eMessageType
tag_invoke(json::value_to_tag<eMessageType>, const json::value& v) {
	return static_cast<eMessageType>(v.to_number<std::underlying_type_t<eMessageType>>());
}

eMessageFlag
tag_invoke(json::value_to_tag<eMessageFlag>, const json::value& v) {
	return static_cast<eMessageFlag>(v.to_number<std::underlying_type_t<eMessageFlag>>());
}

cMessageUpdate
tag_invoke(json::value_to_tag<cMessageUpdate>, const json::value& v) {
	return cMessageUpdate{ v };
}

cMessage
tag_invoke(json::value_to_tag<cMessage>, const json::value& v) {
	return cMessage{ v };
}

cPartialMessageV2::component_type
tag_invoke(json::value_to_tag<cPartialMessageV2::component_type>, const json::value& v) {
	switch (v.at("type").to_number<int>()) {
	case COMPONENT_TEXT_DISPLAY:
		return cPartialMessageV2::component_type(std::in_place_type<cTextDisplay>, v);
	default:
		return cPartialMessageV2::component_type(std::in_place_type<cUnsupportedComponent>, v);
	}
}
/// @}

/** @name JSON value from object conversion
 */
/// @{
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

void
tag_invoke(json::value_from_tag, json::value& v, const cPartialMessage& m) {
	auto& obj = v.emplace_object();
	obj.reserve(5);
	/* Set flags */
	auto flags = m.GetFlags();
	if (flags & MESSAGE_FLAG_TTS)
		obj.emplace("tts", true);
	if ((flags = flags & (MESSAGE_FLAG_EPHEMERAL | MESSAGE_FLAG_SUPPRESS_EMBEDS)))
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

void
tag_invoke(json::value_from_tag, json::value& v, const cPartialMessageV2& m) {
	auto& obj = v.emplace_object();
	obj.reserve(3);

	auto flags = m.GetFlags();
	if (flags & MESSAGE_FLAG_TTS)
		obj.emplace("tts", true);
	obj.emplace("flags", flags & (MESSAGE_FLAG_EPHEMERAL | MESSAGE_FLAG_SUPPRESS_EMBEDS) | MESSAGE_FLAG_IS_COMPONENTS_V2);

	json::value_from(m.GetComponents(), obj["components"]);
}

void
tag_invoke(json::value_from_tag, json::value& v, const cMessage& msg) {
	msg.Visit([&v](const auto& msg) { json::value_from(msg, v); });
}

void
tag_invoke(json::value_from_tag, json::value& v, cMessageView msg) {
	msg.Visit([&v](const auto& msg) { json::value_from(msg, v); });
}

void
tag_invoke(json::value_from_tag, json::value& v, const cPartialMessageV2::component_type& cmp) {
	std::visit([&v](const auto& cmp) { json::value_from(cmp, v); }, cmp);
}
/// @}