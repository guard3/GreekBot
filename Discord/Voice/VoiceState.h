#ifndef _GREEKBOT_VOICESTATE_H_
#define _GREEKBOT_VOICESTATE_H_
#include "Types.h"
#include "Member.h"

class cVoiceState final {
private:
	cSnowflake guild_id;
	cSnowflake channel_id;
	cSnowflake user_id;
	chMember member;
	char* session_id;
	// deaf
	// mute
	// self_deaf
	// self_mute
	// self_stream
	// self_video
	// suppress
	// request_to_speak_timestamp

public:
	explicit cVoiceState(const json::value& v) : channel_id(v.at("channel_id").as_string().c_str()), user_id(v.at("user_id").as_string().c_str()) {
		uchMember m;
		try {
			m = cHandle::MakeUnique<cMember>(v.at("member"));
		}
		catch (...) {}

		try {
			guild_id = v.at("guild_id").as_string().c_str();
		}
		catch (...) {}

		auto& s = v.at("session_id").as_string();
		session_id = new char[s.size() + 1];
		strcpy(session_id, s.c_str());
		member = m.release();
	}
	cVoiceState(const cVoiceState& o) : guild_id(o.guild_id), channel_id(o.channel_id), user_id(o.user_id) {
		session_id = new char[strlen(o.session_id) + 1];
		strcpy(session_id, o.session_id);
		member = new cMember(*o.member);
	}
	cVoiceState(cVoiceState&& o) noexcept : guild_id(o.guild_id), channel_id(o.channel_id), user_id(o.user_id), member(o.member), session_id(o.session_id) {
		o.member = nullptr;
		o.session_id = nullptr;
	}
	~cVoiceState() { delete[] session_id; }

	cVoiceState& operator=(cVoiceState o) {
		guild_id = o.guild_id;
		channel_id = o.channel_id;
		user_id = o.user_id;
		auto t1 = member;
		member = o.member;
		o.member = t1;
		auto t2 = session_id;
		session_id = o.session_id;
		o.session_id = t2;
		return *this;
	}

	chSnowflake GetGuildId()   const { return guild_id.ToInt() == 0 ? nullptr : &guild_id; }
	chSnowflake GetChannelId() const { return &channel_id; }
	chSnowflake GetUserId()    const { return &user_id;    }
	chMember    GetMember()    const { return member;      }
	const char* GetSessionId() const { return session_id;  }
};
typedef   hHandle<cVoiceState>   hVoiceState;
typedef  chHandle<cVoiceState>  chVoiceState;
typedef  uhHandle<cVoiceState>  uhVoiceState;
typedef uchHandle<cVoiceState> uchVoiceState;
typedef  shHandle<cVoiceState>  shVoiceState;
typedef schHandle<cVoiceState> schVoiceState;
#endif /* _GREEKBOT_VOICESTATE_H_ */
