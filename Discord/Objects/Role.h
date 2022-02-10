#pragma once
#ifndef _GREEKBOT_ROLE_H_
#define _GREEKBOT_ROLE_H_
#include "Types.h"
#include "Utils.h"
#include "Discord.h"

enum ePermission : uint64_t {
	PERM_CREATE_INSTANT_INVITE      = 0x0000000000000001, // Allows creation of instant invites
	PERM_KICK_MEMBERS               = 0x0000000000000002, // Allows kicking members
	PERM_BAN_MEMBERS                = 0x0000000000000004, // Allows banning members
	PERM_ADMINISTRATOR              = 0x0000000000000008, // Allows all permissions and bypasses channel permission overwrites
	PERM_MANAGE_CHANNELS            = 0x0000000000000010, // Allows management and editing of channels
	PERM_MANAGE_GUILD               = 0x0000000000000020, // Allows management and editing of the guild
	PERM_ADD_REACTIONS              = 0x0000000000000040, // Allows for the addition of reactions to messages
	PERM_VIEW_AUDIT_LOG             = 0x0000000000000080, // Allows for viewing of audit logs
	PERM_PRIORITY_SPEAKER           = 0x0000000000000100, // Allows for using priority speaker in a voice channel
	PERM_STREAM                     = 0x0000000000000200, // Allows the user to go live
	PERM_VIEW_CHANNEL               = 0x0000000000000400, // Allows guild members to view a channel, which includes reading messages in text channels
	PERM_SEND_MESSAGES              = 0x0000000000000800, // Allows for sending messages in a channel (does not allow sending messages in threads)
	PERM_SEND_TTS_MESSAGES          = 0x0000000000001000, // Allows for sending of /tts messages
	PERM_MANAGE_MESSAGES            = 0x0000000000002000, // Allows for deletion of other users messages
	PERM_EMBED_LINKS                = 0x0000000000004000, // Links sent by users with this permission will be auto-embedded
	PERM_ATTACH_FILES               = 0x0000000000008000, // Allows for uploading images and files
	PERM_READ_MESSAGE_HISTORY       = 0x0000000000010000, // Allows for reading of message history
	PERM_MENTION_EVERYONE           = 0x0000000000020000, // Allows for using the @everyone tag to notify all users in a channel, and the @here tag to notify all online users in a channel
	PERM_USE_EXTERNAL_EMOJIS        = 0x0000000000040000, // Allows the usage of custom emojis from other servers
	PERM_VIEW_GUILD_INSIGHTS        = 0x0000000000080000, // Allows for viewing guild insights
	PERM_CONNECT                    = 0x0000000000100000, // Allows for joining of a voice channel
	PERM_SPEAK                      = 0x0000000000200000, // Allows for speaking in a voice channel
	PERM_MUTE_MEMBERS               = 0x0000000000400000, // Allows for muting members in a voice channel
	PERM_DEAFEN_MEMBERS             = 0x0000000000800000, // Allows for deafening of members in a voice channel
	PERM_MOVE_MEMBERS               = 0x0000000001000000, // Allows for moving of members between voice channels
	PERM_USE_VAD                    = 0x0000000002000000, // Allows for using voice-activity-detection in a voice channel
	PERM_CHANGE_NICKNAME            = 0x0000000004000000, // Allows for modification of own nickname
	PERM_MANAGE_NICKNAMES           = 0x0000000008000000, // Allows for modification of other users nicknames
	PERM_MANAGE_ROLES               = 0x0000000010000000, // Allows management and editing of roles
	PERM_MANAGE_WEBHOOKS            = 0x0000000020000000, // Allows management and editing of webhooks
	PERM_MANAGE_EMOJIS_AND_STICKERS = 0x0000000040000000, // Allows management and editing of emojis and stickers
	PERM_USE_APPLICATION_COMMANDS   = 0x0000000080000000, // Allows members to use application commands, including slash commands and context menu commands.
	PERM_REQUEST_TO_SPEAK           = 0x0000000100000000, // Allows for requesting to speak in stage channels. (This permission is under active development and may be changed or removed.)
	PERM_MANAGE_EVENTS              = 0x0000000200000000, // Allows for creating, editing, and deleting scheduled events
	PERM_MANAGE_THREADS             = 0x0000000400000000, // Allows for deleting and archiving threads, and viewing all private threads
	PERM_CREATE_PUBLIC_THREADS      = 0x0000000800000000, // Allows for creating public and announcement threads
	PERM_CREATE_PRIVATE_THREADS     = 0x0000001000000000, // Allows for creating private threads
	PERM_USE_EXTERNAL_STICKERS      = 0x0000002000000000, // Allows the usage of custom stickers from other servers
	PERM_SEND_MESSAGES_IN_THREADS   = 0x0000004000000000, // Allows for sending messages in threads
	PERM_START_EMBEDDED_ACTIVITIES  = 0x0000008000000000, // Allows for launching activities (applications with the EMBEDDED flag) in a voice channel
	PERM_MODERATE_MEMBERS           = 0x0000010000000000  // Allows for timing out users to prevent them from sending or reacting to messages in chat and threads, and from speaking in voice and stage channels
};
inline ePermission operator|(ePermission a, ePermission b) { return (ePermission)((uint64_t)a | (uint64_t)b); }
inline ePermission operator&(ePermission a, ePermission b) { return (ePermission)((uint64_t)a & (uint64_t)b); }

class cRoleTags final {
private:
	uhSnowflake bot_id;
	uhSnowflake integration_id;
	//premium subscriber

public:
	cRoleTags(const json::value& v) : cRoleTags(v.as_object()) {}
	cRoleTags(const json::object&);
	cRoleTags(const cRoleTags&);
	cRoleTags(cRoleTags&&) = default;

	cRoleTags& operator=(cRoleTags o);

	chSnowflake GetBotId()         const { return bot_id.get();         }
	chSnowflake GetIntegrationId() const { return integration_id.get(); }
};
typedef   hHandle<cRoleTags>   hRoleTags;
typedef  chHandle<cRoleTags>  chRoleTags;
typedef  uhHandle<cRoleTags>  uhRoleTags;
typedef uchHandle<cRoleTags> uchRoleTags;
typedef  shHandle<cRoleTags>  shRoleTags;
typedef schHandle<cRoleTags> schRoleTags;

class cRole final {
private:
	cSnowflake  id;
	std::string name;
	cColor      color;
	bool        hoist;
	std::string icon;
	std::string unicode_emoji;
	int         position;
	ePermission permissions;
	bool        managed;
	bool        mentionable;
	uhRoleTags  tags;

public:
	cRole(const json::value& v) : cRole(v.as_object()) {}
	cRole(const json::object&);
	cRole(const cRole&);
	cRole(cRole&&) = default;

	cRole& operator=(cRole o);

	chSnowflake GetId()           const { return &id;          }
	const char* GetName()         const { return name.c_str(); }
	cColor      GetColor()        const { return color;        }
	int         GetPosition()     const { return position;     }
	ePermission GetPermissions()  const { return permissions;  }
	chRoleTags  GetTags()         const { return tags.get();   }
	const char* GetIconUrl()      const { return icon.empty() ? nullptr : icon.c_str(); }
	const char* GetUnicodeEmoji() const { return unicode_emoji.empty() ? nullptr : unicode_emoji.c_str(); }

	bool IsHoisted()     const { return hoist;       }
	bool IsManaged()     const { return managed;     }
	bool IsMentionable() const { return mentionable; }

	cRole& SetPosition(int v) { position = v; return *this; }
};
typedef   hHandle<cRole>   hRole;
typedef  chHandle<cRole>  chRole;
typedef  uhHandle<cRole>  uhRole;
typedef uchHandle<cRole> uchRole;
typedef  shHandle<cRole>  shRole;
typedef schHandle<cRole> schRole;
#endif /* _GREEKBOT_ROLE_H_ */