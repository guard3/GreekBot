#ifndef DISCORD_VOICESTATE_H
#define DISCORD_VOICESTATE_H
#include "Common.h"
#include "Member.h"
#include <optional>

class cVoiceState final {
	cSnowflake m_guild_id;
	cSnowflake m_channel_id;
	cSnowflake m_user_id;
	std::optional<cMember> m_member;
	std::string m_session_id;
	bool m_deaf;
	bool m_mute;
	bool m_self_deaf;
	bool m_self_mute;
	bool m_self_stream;
	bool m_self_video;
	bool m_suppress;
	std::chrono::sys_time<std::chrono::milliseconds> m_request_to_speak_timestamp;

public:
	explicit cVoiceState(const boost::json::value& v);
	explicit cVoiceState(const boost::json::object& o);

	chSnowflake GetGuildId() const noexcept { return m_guild_id.ToInt() == 0 ? nullptr : &m_guild_id; }
	chSnowflake GetChannelId() const noexcept { return m_channel_id.ToInt() == 0 ? nullptr : &m_channel_id; }
	const cSnowflake& GetUserId() const noexcept { return m_user_id; }
	chMember GetMember() const noexcept { return m_member ? m_member.operator->() : nullptr; }
	hMember GetMember() noexcept { return m_member ? m_member.operator->() : nullptr; }
	std::string_view GetSessionId() const noexcept { return m_session_id; }
	auto GetRequestToSpeakTimestamp() const noexcept { return m_request_to_speak_timestamp; }

	bool IsDeafened() const noexcept { return m_deaf; }
	bool IsMuted() const noexcept { return m_mute; }
	bool IsSelfDeafened() const noexcept { return m_self_deaf; }
	bool IsSelfMuted() const noexcept { return m_self_mute; }
	bool IsStreamingScreen() const noexcept { return m_self_stream; }
	bool IsStreamingCamera() const noexcept { return m_self_video; }
	bool IsSuppressed() const noexcept { return m_suppress; }
};
using   hVoiceState =   hHandle<cVoiceState>;
using  chVoiceState =  chHandle<cVoiceState>;
using  uhVoiceState =  uhHandle<cVoiceState>;
using uchVoiceState = uchHandle<cVoiceState>;

cVoiceState
tag_invoke(boost::json::value_to_tag<cVoiceState>, const boost::json::value& v);
#endif /* DISCORD_VOICESTATE_H */