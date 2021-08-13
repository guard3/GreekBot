#pragma once
#ifndef _GREEKBOT_INTERACTIONRESPONSE_H_
#define _GREEKBOT_INTERACTIONRESPONSE_H_
#include "Types.h"
#include "Component.h"

enum eInteractionCallbackType {
	INTERACTION_CALLBACK_PONG = 1,                                 // ACK a Ping
	INTERACTION_CALLBACK_CHANNEL_MESSAGE_WITH_SOURCE = 4,          // respond to an interaction with a message
	INTERACTION_CALLBACK_DEFERRED_CHANNEL_MESSAGE_WITH_SOURCE = 5, // ACK an interaction and edit a response later, the user sees a loading state
	INTERACTION_CALLBACK_DEFERRED_UPDATE_MESSAGE = 6,              // for components, ACK an interaction and edit the original message later; the user does not see a loading state
	INTERACTION_CALLBACK_UPDATE_MESSAGE = 7                        // for components, edit the message the component was attached to
};



typedef int tInteractionFlag;
inline constexpr tInteractionFlag INTERACTION_FLAG_TTS              = 1 << 0;
inline constexpr tInteractionFlag INTERACTION_FLAG_DISABLE_MENTIONS = 1 << 1;
inline constexpr tInteractionFlag INTERACTION_FLAG_EPHEMERAL        = 1 << 6;

template<eInteractionCallbackType t>
class cInteractionResponse final {
private:
	tInteractionFlag flags;
	std::string content;
	// embeds
	std::vector<cActionRow> components;

public:
	template<typename... Args>
	explicit cInteractionResponse(Args... actionRows) : flags(0), components{ std::move(actionRows)... } {}

	// TODO: Variadic arguments to support embeds as well
	template<typename... Args>
	explicit cInteractionResponse(const char* content, Args... actionRows) : flags(0), content(content ? content : std::string()), components{ std::move(actionRows)... } {}
	template<typename... Args>
	explicit cInteractionResponse(tInteractionFlag flags, Args... actionRows) : flags(flags), components{ std::move(actionRows)... } {}
	template<typename... Args>
	cInteractionResponse(const char* content, tInteractionFlag flags, Args... actionRows) : flags(flags), content(content ? content : std::string()), components{ std::move(actionRows)... } {}

	[[nodiscard]] json::value ToJson() const {
		json::object obj;
		obj["type"] = static_cast<int>(INTERACTION_CALLBACK_CHANNEL_MESSAGE_WITH_SOURCE);
		{
			json::object data;
			if (flags & INTERACTION_FLAG_TTS)
				data["tts"] = true;
			if (!content.empty())
				data["content"] = content;
			if (flags & INTERACTION_FLAG_EPHEMERAL)
				data["flags"] = static_cast<int>(INTERACTION_FLAG_EPHEMERAL);
			if (!components.empty()) {
				json::array a;
				a.reserve(components.size());
				for (auto& c : components)
					a.push_back(c.ToJson());
				data["components"] = std::move(a);
			}
			if (!data.empty())
				obj["data"] = std::move(data);
		}
		return obj;
	}
	[[nodiscard]] std::string ToJsonString() const { return (std::stringstream() << ToJson()).str(); }
};

template<>
class cInteractionResponse<INTERACTION_CALLBACK_DEFERRED_UPDATE_MESSAGE> final {
public:
	[[nodiscard]] std::string ToJsonString() const {
		std::string result(R"({"type": })");
		result[8] = INTERACTION_CALLBACK_DEFERRED_UPDATE_MESSAGE + '0';
		return result;
	}
};

template<>
class cInteractionResponse<INTERACTION_CALLBACK_DEFERRED_CHANNEL_MESSAGE_WITH_SOURCE> final {
public:
	[[nodiscard]] std::string ToJsonString() const {
		std::string result(R"({"type": })");
		result[8] = INTERACTION_CALLBACK_DEFERRED_CHANNEL_MESSAGE_WITH_SOURCE + '0';
		return result;
	}
};

#endif /* _GREEKBOT_INTERACTIONRESPONSE_H_ */
