#ifndef DISCORD_VOICESTATE_H
#define DISCORD_VOICESTATE_H
#include "Member.h"
#include "VoiceStateFwd.h"
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

	auto&&  GetUserId(this auto&& self) noexcept { return std::forward<decltype(self)>(self).m_user_id; }
	auto   GetGuildId(this auto&  self) noexcept { return cPtr(self.m_guild_id.ToInt()   == 0 ? nullptr : &self.m_guild_id    ); }
	auto GetChannelId(this auto&  self) noexcept { return cPtr(self.m_channel_id.ToInt() == 0 ? nullptr : &self.m_channel_id); }
	auto    GetMember(this auto&  self) noexcept { return cPtr(self.m_member ? self.m_member.operator->() : nullptr); }
	std::string_view   GetSessionId() const noexcept { return m_session_id;                 }
	auto GetRequestToSpeakTimestamp() const noexcept { return m_request_to_speak_timestamp; }

	bool        IsDeafened() const noexcept { return m_deaf;        }
	bool           IsMuted() const noexcept { return m_mute;        }
	bool    IsSelfDeafened() const noexcept { return m_self_deaf;   }
	bool       IsSelfMuted() const noexcept { return m_self_mute;   }
	bool IsStreamingScreen() const noexcept { return m_self_stream; }
	bool IsStreamingCamera() const noexcept { return m_self_video;  }
	bool      IsSuppressed() const noexcept { return m_suppress;    }
};

cVoiceState
tag_invoke(boost::json::value_to_tag<cVoiceState>, const boost::json::value& v);
#endif /* DISCORD_VOICESTATE_H */