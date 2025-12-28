#ifndef DISCORD_PERMISSION_H
#define DISCORD_PERMISSION_H
#include "Base.h"

struct ePermission final : cFlagSet<std::uint64_t> {
	explicit constexpr ePermission(cFlagSet s = 0) noexcept : cFlagSet(s) {}
};

constexpr ePermission PERM_CREATE_INSTANT_INVITE      { 0x0000000000000001 }; // Allows creation of instant invites
constexpr ePermission PERM_KICK_MEMBERS               { 0x0000000000000002 }; // Allows kicking members
constexpr ePermission PERM_BAN_MEMBERS                { 0x0000000000000004 }; // Allows banning members
constexpr ePermission PERM_ADMINISTRATOR              { 0x0000000000000008 }; // Allows all permissions and bypasses channel permission overwrites
constexpr ePermission PERM_MANAGE_CHANNELS            { 0x0000000000000010 }; // Allows management and editing of channels
constexpr ePermission PERM_MANAGE_GUILD               { 0x0000000000000020 }; // Allows management and editing of the guild
constexpr ePermission PERM_ADD_REACTIONS              { 0x0000000000000040 }; // Allows for the addition of reactions to messages
constexpr ePermission PERM_VIEW_AUDIT_LOG             { 0x0000000000000080 }; // Allows for viewing of audit logs
constexpr ePermission PERM_PRIORITY_SPEAKER           { 0x0000000000000100 }; // Allows for using priority speaker in a voice channel
constexpr ePermission PERM_STREAM                     { 0x0000000000000200 }; // Allows the user to go live
constexpr ePermission PERM_VIEW_CHANNEL               { 0x0000000000000400 }; // Allows guild members to view a channel, which includes reading messages in text channels
constexpr ePermission PERM_SEND_MESSAGES              { 0x0000000000000800 }; // Allows for sending messages in a channel (does not allow sending messages in threads)
constexpr ePermission PERM_SEND_TTS_MESSAGES          { 0x0000000000001000 }; // Allows for sending of /tts messages
constexpr ePermission PERM_MANAGE_MESSAGES            { 0x0000000000002000 }; // Allows for deletion of other users messages
constexpr ePermission PERM_EMBED_LINKS                { 0x0000000000004000 }; // Links sent by users with this permission will be auto-embedded
constexpr ePermission PERM_ATTACH_FILES               { 0x0000000000008000 }; // Allows for uploading images and files
constexpr ePermission PERM_READ_MESSAGE_HISTORY       { 0x0000000000010000 }; // Allows for reading of message history
constexpr ePermission PERM_MENTION_EVERYONE           { 0x0000000000020000 }; // Allows for using the @everyone tag to notify all users in a channel, and the @here tag to notify all online users in a channel
constexpr ePermission PERM_USE_EXTERNAL_EMOJIS        { 0x0000000000040000 }; // Allows the usage of custom emojis from other servers
constexpr ePermission PERM_VIEW_GUILD_INSIGHTS        { 0x0000000000080000 }; // Allows for viewing guild insights
constexpr ePermission PERM_CONNECT                    { 0x0000000000100000 }; // Allows for joining of a voice channel
constexpr ePermission PERM_SPEAK                      { 0x0000000000200000 }; // Allows for speaking in a voice channel
constexpr ePermission PERM_MUTE_MEMBERS               { 0x0000000000400000 }; // Allows for muting members in a voice channel
constexpr ePermission PERM_DEAFEN_MEMBERS             { 0x0000000000800000 }; // Allows for deafening of members in a voice channel
constexpr ePermission PERM_MOVE_MEMBERS               { 0x0000000001000000 }; // Allows for moving of members between voice channels
constexpr ePermission PERM_USE_VAD                    { 0x0000000002000000 }; // Allows for using voice-activity-detection in a voice channel
constexpr ePermission PERM_CHANGE_NICKNAME            { 0x0000000004000000 }; // Allows for modification of own nickname
constexpr ePermission PERM_MANAGE_NICKNAMES           { 0x0000000008000000 }; // Allows for modification of other users nicknames
constexpr ePermission PERM_MANAGE_ROLES               { 0x0000000010000000 }; // Allows management and editing of roles
constexpr ePermission PERM_MANAGE_WEBHOOKS            { 0x0000000020000000 }; // Allows management and editing of webhooks
constexpr ePermission PERM_MANAGE_EMOJIS_AND_STICKERS { 0x0000000040000000 }; // Allows management and editing of emojis and stickers
constexpr ePermission PERM_USE_APPLICATION_COMMANDS   { 0x0000000080000000 }; // Allows members to use application commands, including slash commands and context menu commands.
constexpr ePermission PERM_REQUEST_TO_SPEAK           { 0x0000000100000000 }; // Allows for requesting to speak in stage channels. (This permission is under active development and may be changed or removed.)
constexpr ePermission PERM_MANAGE_EVENTS              { 0x0000000200000000 }; // Allows for creating, editing, and deleting scheduled events
constexpr ePermission PERM_MANAGE_THREADS             { 0x0000000400000000 }; // Allows for deleting and archiving threads, and viewing all private threads
constexpr ePermission PERM_CREATE_PUBLIC_THREADS      { 0x0000000800000000 }; // Allows for creating public and announcement threads
constexpr ePermission PERM_CREATE_PRIVATE_THREADS     { 0x0000001000000000 }; // Allows for creating private threads
constexpr ePermission PERM_USE_EXTERNAL_STICKERS      { 0x0000002000000000 }; // Allows the usage of custom stickers from other servers
constexpr ePermission PERM_SEND_MESSAGES_IN_THREADS   { 0x0000004000000000 }; // Allows for sending messages in threads
constexpr ePermission PERM_START_EMBEDDED_ACTIVITIES  { 0x0000008000000000 }; // Allows for launching activities (applications with the EMBEDDED flag) in a voice channel
constexpr ePermission PERM_MODERATE_MEMBERS           { 0x0000010000000000 }; // Allows for timing out users to prevent them from sending or reacting to messages in chat and threads, and from speaking in voice and stage channels

ePermission tag_invoke(boost::json::value_to_tag<ePermission>, const boost::json::value&);
#endif //DISCORD_PERMISSION_H