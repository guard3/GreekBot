#include "Utils.h"
#include "VoiceState.h"
#include <boost/json.hpp>

namespace json = boost::json;

cVoiceState::cVoiceState(const json::value& v): cVoiceState(v.as_object()) {}
cVoiceState::cVoiceState(const json::object& o):
	m_guild_id([&o] {
		auto p = o.if_contains("guild_id");
		return p ? cSnowflake(p->as_string()) : cSnowflake{};
	}()),
	m_channel_id([&o] {
		auto& p = o.at("channel_id");
		return p.is_null() ? cSnowflake{} : cSnowflake(p.as_string());
	}()),
	m_user_id(o.at("user_id").as_string()),
	m_session_id(json::value_to<std::string>(o.at("session_id"))),
	m_deaf(o.at("deaf").as_bool()),
	m_mute(o.at("mute").as_bool()),
	m_self_deaf(o.at("self_deaf").as_bool()),
	m_self_mute(o.at("self_mute").as_bool()),
	m_self_video(o.at("self_video").as_bool()),
	m_suppress(o.at("suppress").as_bool()) {
	auto p = o.if_contains("member");
	if (p)
		m_member.emplace(*p);
	p = o.if_contains("self_stream");
	m_self_stream = p && p->as_bool();
	auto& v = o.at("request_to_speak_timestamp");
	if (!v.is_null())
		m_request_to_speak_timestamp = cUtils::ParseISOTimestamp(v.as_string());
}

cVoiceState
tag_invoke(json::value_to_tag<cVoiceState>, const json::value& v) {
	return cVoiceState{ v };
}