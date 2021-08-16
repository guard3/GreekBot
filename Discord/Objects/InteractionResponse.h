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

enum eInteractionFlag {
	INTERACTION_FLAG_TTS               = 1 << 0,
	INTERACTION_FLAG_DISABLE_MENTIONS  = 1 << 1,
	INTERACTION_FLAG_REMOVE_COMPONENTS = 1 << 2,
	INTERACTION_FLAG_EPHEMERAL         = 1 << 6
};
inline eInteractionFlag operator|(eInteractionFlag f1, eInteractionFlag f2) { return static_cast<eInteractionFlag>(static_cast<int>(f1) | static_cast<int>(f2)); }

class cBaseInteractionResponse {
private:
	eInteractionCallbackType type;

public:
	explicit cBaseInteractionResponse(eInteractionCallbackType t) : type(t) {}

	[[nodiscard]] eInteractionCallbackType GetCallbackType() const { return type; }

	[[nodiscard]] virtual json::object ToJson() const {
		json::object obj;
		obj["type"] = static_cast<int>(type);
		return obj;
	};
	[[nodiscard]] std::string ToJsonString() const { return (std::stringstream() << ToJson()).str(); }
};

template<eInteractionCallbackType t>
class cInteractionResponse final : public cBaseInteractionResponse {
private:
	eInteractionFlag flags;
	std::string content;
	// embeds
	std::vector<cActionRow> components;

public:
	template<typename... Args>
	cInteractionResponse(const char* content, eInteractionFlag flags, Args... actionRows) : cBaseInteractionResponse(t), flags(flags), content(content ? content : std::string()), components{ std::move(actionRows)... } {}
	template<typename... Args>
	explicit cInteractionResponse(Args&&... actionRows) : cInteractionResponse(nullptr, static_cast<eInteractionFlag>(0), std::forward<Args>(actionRows)...) {}
	template<typename... Args>
	explicit cInteractionResponse(eInteractionFlag flags, Args&&... actionRows) : cInteractionResponse(nullptr, flags, std::forward<Args>(actionRows)...)  {}
	template<typename... Args>
	explicit cInteractionResponse(const char* content, Args&&... actionRows) : cInteractionResponse(content, static_cast<eInteractionFlag>(0), std::forward<Args>(actionRows)...) {}

	[[nodiscard]] json::object ToJson() const override {
		json::object obj;
		obj["type"] = static_cast<int>(t);
		{
			json::object data;
			if (flags & INTERACTION_FLAG_TTS)
				data["tts"] = true;
			if (!content.empty())
				data["content"] = content;
			if (flags & INTERACTION_FLAG_EPHEMERAL)
				data["flags"] = static_cast<int>(INTERACTION_FLAG_EPHEMERAL);
			if (components.empty()) {
				if (flags & INTERACTION_FLAG_REMOVE_COMPONENTS)
					data["components"] = json::array();
			}
			else {
				json::array a;
				a.reserve(components.size());
				for (auto& c : components)
					a.push_back(c.ToJson());
				data["components"] = std::move(a);
			}
			obj["data"] = std::move(data);
		}
		return obj;
	}
};

template<>
class cInteractionResponse<INTERACTION_CALLBACK_DEFERRED_UPDATE_MESSAGE> final : public cBaseInteractionResponse {
public:
	cInteractionResponse() : cBaseInteractionResponse(INTERACTION_CALLBACK_DEFERRED_UPDATE_MESSAGE) {}
};

template<>
class cInteractionResponse<INTERACTION_CALLBACK_DEFERRED_CHANNEL_MESSAGE_WITH_SOURCE> final : public cBaseInteractionResponse {
public:
	cInteractionResponse() : cBaseInteractionResponse(INTERACTION_CALLBACK_DEFERRED_CHANNEL_MESSAGE_WITH_SOURCE) {}
};

#endif /* _GREEKBOT_INTERACTIONRESPONSE_H_ */
