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

enum eMessageFlag {
	MESSAGE_FLAG_NONE                                   = 0,
	MESSAGE_FLAG_CROSSPOSTED                            = 1 << 0, // this message has been published to subscribed channels (via Channel Following)
	MESSAGE_FLAG_IS_CROSSPOST                           = 1 << 1, // this message originated from a message in another channel (via Channel Following)
	MESSAGE_FLAG_SUPPRESS_EMBEDS                        = 1 << 2, // do not include any embeds when serializing this message
	MESSAGE_FLAG_SOURCE_MESSAGE_DELETED                 = 1 << 3, // the source message for this crosspost has been deleted (via Channel Following)
	MESSAGE_FLAG_URGENT                                 = 1 << 4, // this message came from the urgent message system
	MESSAGE_FLAG_HAS_THREAD                             = 1 << 5, // this message has an associated thread, with the same id as the message
	MESSAGE_FLAG_EPHEMERAL                              = 1 << 6, // this message is only visible to the user who invoked the Interaction
	MESSAGE_FLAG_LOADING                                = 1 << 7, // this message is an Interaction Response and the bot is "thinking"
	MESSAGE_FLAG_FAILED_TO_MENTION_SOME_ROLES_IN_THREAD = 1 << 8, // this message failed to mention some roles and add their members to the thread

	/* Custom flags */
	MESSAGE_FLAG_TTS              = 1 << 15, // this is a TTS message
	MESSAGE_FLAG_MENTION_EVERYONE = 1 << 16  // this message mentions everyone
};
inline eMessageFlag operator|(eMessageFlag a, eMessageFlag b) { return (eMessageFlag)((int)a | (int)b); }
inline eMessageFlag operator&(eMessageFlag a, eMessageFlag b) { return (eMessageFlag)((int)a & (int)b); }

class cMessage final {
private:
	cSnowflake  id;
	cSnowflake  channel_id;
	uhSnowflake guild_id;
	cUser       author;
	// member
	std::string content;
	std::string timestamp;
	// edited_timestamp
	//bool tts;
	// ...
	eMessageType type;
	// ...
	eMessageFlag flags;

public:
	//template<typename T, typename = std::enable_if_t<std::is_same_v<T, json::value>>>
	explicit cMessage(const json::value& v):
		id(v.at("id")),
		channel_id(v.at("channel_id")),
		guild_id(cHandle::MakeUniqueNoEx<cSnowflake>(v.at("guild_id"))),
		author(v.at("author")),
		content(v.at("content").as_string().c_str()),
		timestamp(v.at("timestamp").as_string().c_str()),
		type((eMessageType)v.at("type").as_int64())
	{
		const json::value* f;
		flags = (eMessageFlag)((f = v.as_object().if_contains("flags")) ? f->as_int64() : 0);
		flags = flags | (eMessageFlag)((v.at("tts").as_bool() << 15) | (v.at("mention_everyone").as_bool() << 16));
	}
	chSnowflake  GetId()        const { return &id;               }
	chSnowflake  GetChannelId() const { return &channel_id;       }
	chSnowflake  GetGuildId()   const { return guild_id.get();    }
	chUser       GetAuthor()    const { return &author;           }
	const char  *GetContent()   const { return content.c_str();   }
	const char  *GetTimestamp() const { return timestamp.c_str(); }
	eMessageType GetType()      const { return type;              }
	eMessageFlag GetFlags()     const { return flags;             }
};
typedef   hHandle<cMessage>   hMessage;
typedef  chHandle<cMessage>  chMessage;
typedef  uhHandle<cMessage>  uhMessage;
typedef uchHandle<cMessage> uchMessage;
typedef  shHandle<cMessage>  shMessage;
typedef schHandle<cMessage> schMessage;

#endif /* _GREEKBOT_MESSAGE_H_ */
