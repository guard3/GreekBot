#pragma once
#ifndef _GREEKBOT_MESSAGE_H_
#define _GREEKBOT_MESSAGE_H_
#include "User.h"
#include "Embed.h"
#include "Member.h"

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
	uhMember    member;
	std::string content;
	std::string timestamp;
	std::string edited_timestamp;
	// ...
	eMessageType type;
	// ...
	eMessageFlag flags;

public:
	std::vector<cEmbed> Embeds;

	cMessage(const json::object&);
	cMessage(const json::value& v);
	cMessage(const cMessage& o);
	cMessage(cMessage&&) = default;

	cMessage& operator=(cMessage o);

	const cSnowflake&  GetId()              const { return id;               }
	const cSnowflake&  GetChannelId()       const { return channel_id;       }
	chSnowflake        GetGuildId()         const { return guild_id.get();   }
	const cUser&       GetAuthor()          const { return author;           }
	chMember           GetMember()          const { return member.get();     }
	const std::string& GetContent()         const { return content;          }
	const std::string& GetTimestamp()       const { return timestamp;        }
	const std::string& GetEditedTimestamp() const { return edited_timestamp; }
	eMessageType       GetType()            const { return type;             }
	eMessageFlag       GetFlags()           const { return flags;            }
};
typedef   hHandle<cMessage>   hMessage;
typedef  chHandle<cMessage>  chMessage;
typedef  uhHandle<cMessage>  uhMessage;
typedef uchHandle<cMessage> uchMessage;
typedef  shHandle<cMessage>  shMessage;
typedef schHandle<cMessage> schMessage;
#endif /* _GREEKBOT_MESSAGE_H_ */