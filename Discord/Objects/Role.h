#ifndef DISCORD_ROLE_H
#define DISCORD_ROLE_H
#include "Base.h"
#include "RoleFwd.h"
#include <optional>

enum ePermission : std::uint64_t {
	PERM_NONE = 0,
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

ePermission tag_invoke(boost::json::value_to_tag<ePermission>, const boost::json::value&);

class cRoleTags final {
	cSnowflake bot_id;
	cSnowflake integration_id;
	// various other crap...

public:
	cRoleTags(const boost::json::value&);
	cRoleTags(const boost::json::object&);

	chSnowflake         GetBotId() const noexcept { return         bot_id.ToInt() ?         &bot_id : nullptr; }
	chSnowflake GetIntegrationId() const noexcept { return integration_id.ToInt() ? &integration_id : nullptr; }
};

class cRole final {
	cSnowflake  m_id;
	std::size_t m_position;
	ePermission m_permissions;
	cColor      m_color;
	bool        m_hoist;
	bool        m_managed;
	bool        m_mentionable;
	std::string m_name;
	std::string m_icon;
	std::string m_unicode_emoji;
	std::optional<cRoleTags> m_tags;

public:
	cRole(const boost::json::value&);
	cRole(const boost::json::object&);

	const cSnowflake&          GetId() const noexcept { return m_id;            }
	std::size_t          GetPosition() const noexcept { return m_position;      }
	ePermission       GetPermissions() const noexcept { return m_permissions;   }
	cColor                  GetColor() const noexcept { return m_color;         }
	std::string_view         GetName() const noexcept { return m_name;          }
	std::string_view         GetIcon() const noexcept { return m_icon;          }
	std::string_view GetUnicodeEmoji() const noexcept { return m_unicode_emoji; }
	chRoleTags               GetTags() const noexcept { return m_tags ? m_tags.operator->() : nullptr; }

	bool     IsHoisted() const noexcept { return m_hoist;       }
	bool     IsManaged() const noexcept { return m_managed;     }
	bool IsMentionable() const noexcept { return m_mentionable; }
};

cRole tag_invoke(boost::json::value_to_tag<cRole>, const boost::json::value&);
#endif /* DISCORD_ROLE_H */