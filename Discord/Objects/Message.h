#pragma once
#ifndef _GREEKBOT_MESSAGE_H_
#define _GREEKBOT_MESSAGE_H_
#include "User.h"

enum eMessageType {
	MESSAGE_TYPE_DEFAULT,
	MESSAGE_TYPE_RECIPIENT_ADD,
	MESSAGE_TYPE_RECIPIENT_REMOVE,
	MESSAGE_TYPE_CALL,
	MESSAGE_TYPE_CHANNEL_NAME_CHANGE,
	MESSAGE_TYPE_CHANNEL_ICON_CHANGE,
	MESSAGE_TYPE_CHANNEL_PINNED_MESSAGE,
	MESSAGE_TYPE_GUILD_MEMBER_JOIN,
	MESSAGE_TYPE_USER_PREMIUM_GUILD_SUBSCRIPTION,
	MESSAGE_TYPE_USER_PREMIUM_GUILD_SUBSCRIPTION_TIER_1,
	MESSAGE_TYPE_USER_PREMIUM_GUILD_SUBSCRIPTION_TIER_2,
	MESSAGE_TYPE_USER_PREMIUM_GUILD_SUBSCRIPTION_TIER_3,
	MESSAGE_TYPE_CHANNEL_FOLLOW_ADD,
	// 13
	MESSAGE_TYPE_GUILD_DISCOVERY_DISQUALIFIED = 14,
	MESSAGE_TYPE_GUILD_DISCOVERY_REQUALIFIED,
	MESSAGE_TYPE_GUILD_DISCOVERY_GRACE_PERIOD_INITIAL_WARNING,
	MESSAGE_TYPE_GUILD_DISCOVERY_GRACE_PERIOD_FINAL_WARNING,
	MESSAGE_TYPE_THREAD_CREATED,
	MESSAGE_TYPE_REPLY,
	MESSAGE_TYPE_CHAT_INPUT_COMMAND,
	MESSAGE_TYPE_THREAD_STARTER_MESSAGE,
	MESSAGE_TYPE_GUILD_INVITE_REMINDER,
	MESSAGE_TYPE_CONTEXT_MENU_COMMAND
};

class cMessage final {
private:
	cSnowflake  id;
	cSnowflake  channel_id;
	uhSnowflake guild_id;
	cUser       author;
	// member
	json::string content;
	json::string timestamp;
	// edited_timestamp
	bool tts;
	// ...
	eMessageType type;
	// ...
public:
	//template<typename T, typename = std::enable_if_t<std::is_same_v<T, json::value>>>
	explicit cMessage(const json::value& v):
		id(v.at("id")),
		channel_id(v.at("channel_id")),
		guild_id(cHandle::MakeUniqueNoEx<cSnowflake>(v.at("guild_id"))),
		author(v.at("author")),
		content(std::move(v.at("content").as_string())),
		timestamp(std::move(v.at("timestamp").as_string())),
		tts(v.at("tts").as_bool()),
		type((eMessageType)v.at("type").as_int64())
	{
	}
	chSnowflake  GetId()        const { return &id;               }
	chSnowflake  GetChannelId() const { return &channel_id;       }
	chSnowflake  GetGuildId()   const { return guild_id.get();    }
	chUser       GetAuthor()    const { return &author;           }
	const char  *GetContent()   const { return content.c_str();   }
	const char  *GetTimestamp() const { return timestamp.c_str(); }
	eMessageType GetType()      const { return type;              }

	bool IsTTS() const { return tts; }
};
typedef   hHandle<cMessage>   hMessage;
typedef  chHandle<cMessage>  chMessage;
typedef  uhHandle<cMessage>  uhMessage;
typedef uchHandle<cMessage> uchMessage;
typedef  shHandle<cMessage>  shMessage;
typedef schHandle<cMessage> schMessage;

#endif /* _GREEKBOT_MESSAGE_H_ */
