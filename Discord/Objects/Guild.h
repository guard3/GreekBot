#ifndef _GREEKBOT_GUILD_H_
#define _GREEKBOT_GUILD_H_
#include "Types.h"
#include "Channel.h"
#include "VoiceState.h"

class cGuild final {
private:
	cSnowflake id;
	std::vector<cChannel> channels;
	std::vector<cVoiceState> voice_states;
public:
	std::vector<chChannel> Channels;
	std::vector<chVoiceState> VoiceStates;

	explicit cGuild(const json::value& v) : id(v.at("id").as_string().c_str()) {
		try {
			auto& a = v.at("channels").as_array();
			channels.reserve(a.size());
			Channels.reserve(a.size());
			for (auto& o : a) {
				channels.emplace_back(o);
				Channels.push_back(&channels.back());
			}
		}
		catch (...) {
			channels.clear();
			Channels.clear();
		}
		try {
			auto& a = v.at("voice_states").as_array();
			voice_states.reserve(a.size());
			VoiceStates.reserve(a.size());
			for (auto& o : a) {
				voice_states.emplace_back(o);
				VoiceStates.push_back(&voice_states.back());
			}
		}
		catch (...) {
			voice_states.clear();
			VoiceStates.clear();
		}
	}
	cGuild(const cGuild& o) : id(o.id), channels(o.channels), voice_states(o.voice_states) {
		Channels.reserve(channels.size());
		for (auto& a : channels)
			Channels.push_back(&a);
		VoiceStates.reserve(voice_states.size());
		for (auto& a : voice_states)
			VoiceStates.push_back(&a);
	}
	cGuild(cGuild&& o) noexcept : id(o.id), channels(std::move(o.channels)), voice_states(std::move(o.voice_states)), Channels(std::move(o.Channels)), VoiceStates(std::move(o.VoiceStates)) {}

	cGuild& operator=(cGuild o) {
		id = o.id;
		channels.swap(o.channels);
		Channels.swap(o.Channels);
		voice_states.swap(o.voice_states);
		VoiceStates.swap(o.VoiceStates);
		return *this;
	}

	chSnowflake GetId() const { return &id; }
};
typedef   hHandle<cGuild>   hGuild;
typedef  chHandle<cGuild>  chGuild;
typedef  uhHandle<cGuild>  uhGuild;
typedef uchHandle<cGuild> uchGuild;
typedef  shHandle<cGuild>  shGuild;
typedef schHandle<cGuild> schGuild;
#endif /* _GREEKBOT_GUILD_H_ */
