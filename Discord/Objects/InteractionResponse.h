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

template<eInteractionCallbackType t>
class cInteractionResponse final {
public:
	[[nodiscard]] std::string ToJsonString() const {
		std::string result(R"({"type": })");
		result[8] = t + '0';
		return result;
	}
};

template<>
class cInteractionResponse<INTERACTION_CALLBACK_CHANNEL_MESSAGE_WITH_SOURCE> final {
private:
	bool tts = false;
	std::string content;
	// embeds
	// allowed_mentions
	bool ephemeral = false;
	// components
	cActionRow meow;

public:
	cInteractionResponse() = default;
	// TODO: Variadic arguments to support embeds as well
	cInteractionResponse(const std::string& str) : content(str) {}

	cInteractionResponse& SetContent(const std::string& str) {
		content = str;
		return *this;
	}
	cInteractionResponse& SetTTS(bool b) {
		tts = b;
		return *this;
	}
	cInteractionResponse& SetEphemeral(bool b) {
		ephemeral = b;
		return *this;
	}

	template<eButtonStyle s>
	cInteractionResponse& SetComponent(const cButton<s>& b) {
		meow = cActionRow { b };
		return *this;
	}

	[[nodiscard]] json::value ToJson() const {
		json::object obj;
		obj["type"] = static_cast<int>(INTERACTION_CALLBACK_CHANNEL_MESSAGE_WITH_SOURCE);
		json::object data;
		if (tts)
			data["tts"] = true;
		if (!content.empty())
			data["content"] = content;
		if (ephemeral)
			data["flags"] = 0x40;
		json::array components;
		components.push_back(meow.ToJson());
		data["components"] = components;
		if (!data.empty())
			obj["data"] = data;

		return obj;
	}

	[[nodiscard]] std::string ToJsonString() const { return (std::stringstream() << ToJson()).str(); }
};

#endif /* _GREEKBOT_INTERACTIONRESPONSE_H_ */
