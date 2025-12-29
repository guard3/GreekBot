#ifndef DISCORD_PERMISSION_H
#define DISCORD_PERMISSION_H
#include "Base.h"
#include <bit>

struct ePermission final : cFlagSet<std::uint64_t> {
	explicit constexpr ePermission(cFlagSet s = 0) noexcept : cFlagSet(s) {}

	static const ePermission CreateInstantInvite;              // 0x0000000000000001 Allows creation of instant invites
	static const ePermission KickMembers;                      // 0x0000000000000002 Allows kicking members
	static const ePermission BanMembers;                       // 0x0000000000000004 Allows banning members
	static const ePermission Administrator;                    // 0x0000000000000008 Allows all permissions and bypasses channel permission overwrites
	static const ePermission ManageChannels;                   // 0x0000000000000010 Allows management and editing of channels
	static const ePermission ManageGuild;                      // 0x0000000000000020 Allows management and editing of the guild
	static const ePermission AddReactions;                     // 0x0000000000000040 Allows for adding new reactions to messages. This permission does not apply to reacting with an existing reaction on a message.
	static const ePermission ViewAuditLog;                     // 0x0000000000000080 Allows for viewing of audit logs
	static const ePermission PrioritySpeaker;                  // 0x0000000000000100 Allows for using priority speaker in a voice channel
	static const ePermission Stream;                           // 0x0000000000000200 Allows the user to go live
	static const ePermission ViewChannel;                      // 0x0000000000000400 Allows guild members to view a channel, which includes reading messages in text channels and joining voice channels
	static const ePermission SendMessages;                     // 0x0000000000000800 Allows for sending messages in a channel and creating threads in a forum (does not allow sending messages in threads)
	static const ePermission SendTTSMessages;                  // 0x0000000000001000 Allows for sending of /tts messages
	static const ePermission ManageMessages;                   // 0x0000000000002000 Allows for deletion of other users messages
	static const ePermission EmbedLinks;                       // 0x0000000000004000 Links sent by users with this permission will be auto-embedded
	static const ePermission AttachFiles;                      // 0x0000000000008000 Allows for uploading images and files
	static const ePermission ReadMessageHistory;               // 0x0000000000010000 Allows for reading of message history
	static const ePermission MentionEveryone;                  // 0x0000000000020000 Allows for using the @everyone tag to notify all users in a channel, and the @here tag to notify all online users in a channel
	static const ePermission UseExternalEmojis;                // 0x0000000000040000 Allows the usage of custom emojis from other servers
	static const ePermission ViewGuildInsights;                // 0x0000000000080000 Allows for viewing guild insights
	static const ePermission Connect;                          // 0x0000000000100000 Allows for joining of a voice channel
	static const ePermission Speak;                            // 0x0000000000200000 Allows for speaking in a voice channel
	static const ePermission MuteMembers;                      // 0x0000000000400000 Allows for muting members in a voice channel
	static const ePermission DeafenMembers;                    // 0x0000000000800000 Allows for deafening of members in a voice channel
	static const ePermission MoveMembers;                      // 0x0000000001000000 Allows for moving of members between voice channels
	static const ePermission UseVAD;                           // 0x0000000002000000 Allows for using voice-activity-detection in a voice channel
	static const ePermission ChangeNickname;                   // 0x0000000004000000 Allows for modification of own nickname
	static const ePermission ManageNicknames;                  // 0x0000000008000000 Allows for modification of other users nicknames
	static const ePermission ManageRoles;                      // 0x0000000010000000 Allows management and editing of roles
	static const ePermission ManageWebhooks;                   // 0x0000000020000000 Allows management and editing of webhooks
	static const ePermission ManageGuildExpressions;           // 0x0000000040000000 Allows for editing and deleting emojis, stickers, and soundboard sounds created by all users
	static const ePermission UseApplicationCommands;           // 0x0000000080000000 Allows members to use application commands, including slash commands and context menu commands.
	static const ePermission RequestToSpeak;                   // 0x0000000100000000 Allows for requesting to speak in stage channels. (This permission is under active development and may be changed or removed.)
	static const ePermission ManageEvents;                     // 0x0000000200000000 Allows for editing and deleting scheduled events created by all users
	static const ePermission ManageThreads;                    // 0x0000000400000000 Allows for deleting and archiving threads, and viewing all private threads
	static const ePermission CreatePublicThreads;              // 0x0000000800000000 Allows for creating public and announcement threads
	static const ePermission CreatePrivateThreads;             // 0x0000001000000000 Allows for creating private threads
	static const ePermission UseExternalStickers;              // 0x0000002000000000 Allows the usage of custom stickers from other servers
	static const ePermission SendMessagesInThreads;            // 0x0000004000000000 Allows for sending messages in threads
	static const ePermission UseEmbeddedActivities;            // 0x0000008000000000 Allows for using Activities (applications with the EMBEDDED flag)
	static const ePermission ModerateMembers;                  // 0x0000010000000000 Allows for timing out users to prevent them from sending or reacting to messages in chat and threads, and from speaking in voice and stage channels
	static const ePermission ViewCreatorMonetizationAnalytics; // 0x0000020000000000 Allows for viewing role subscription insights
	static const ePermission UseSoundboard;                    // 0x0000040000000000 Allows for using soundboard in a voice channel
	static const ePermission CreateGuildExpressions;           // 0x0000080000000000 Allows for creating emojis, stickers, and soundboard sounds, and editing and deleting those created by the current user.
	static const ePermission CreateEvents;                     // 0x0000100000000000 Allows for creating scheduled events, and editing and deleting those created by the current user.
	static const ePermission UseExternalSounds;                // 0x0000200000000000 Allows the usage of custom soundboard sounds from other servers
	static const ePermission SendVoiceMessages;                // 0x0000400000000000 Allows sending voice messages
	///////////////////////////////////////////////////////////// 0x0000800000000000 Unused
	///////////////////////////////////////////////////////////// 0x0001000000000000 Unused
	static const ePermission SendPolls;                        // 0x0002000000000000 Allows sending polls
	static const ePermission UseExternalApps;                  // 0x0004000000000000 Allows user-installed apps to send public responses. When disabled, users will still be allowed to use their apps but the responses will be ephemeral. This only applies to apps not also installed to the server.
	static const ePermission PinMessages;                      // 0x0008000000000000 Allows pinning and unpinning messages
	static const ePermission BypassSlowmode;                   // 0x0010000000000000 Allows bypassing slowmode restrictions

	/**
	 * Formats the permission value to a | delimited string of each permission name to a range beginning at \p out
	 * @param out The output iterator that denotes the beginning of the output range
	 * @return Iterator past the end of the output range
	 */
	template<std::output_iterator<char> It>
	It FormatString(this ePermission self, It out) {
		// Handle the special case of Administrator
		if (self.TestAny(Administrator)) {
			const auto name = ms_names[std::countr_zero(std::bit_cast<std::uint64_t>(Administrator))];
			return std::copy(name.begin(), name.end(), out);
		}

		// Copy each flag name to out, separating multiple values with |
		const auto perm = std::bit_cast<std::uint64_t>(self);
		const char* sep_begin = "", *sep_end = sep_begin;
		for (std::uint64_t i = 0; i < 64; ++i) {
			if (perm & ((std::uint64_t)1 << i)) {
				out = std::copy(sep_begin, sep_end, out);

				const auto name = ms_names[i];
				out = name.empty() ? std::format_to(out, "UNKNOWN_{}", i) : std::copy(name.begin(), name.end(), out);

				sep_begin = " | ", sep_end = sep_begin + 3;
			}
		}

		return out;
	}

	/**
	 * Creates a | delimited string representation of the permission value with each permission name
	 * @return A string describing the permission value
	 */
	std::string ToString(this ePermission self) {
		std::string str;
		str.reserve(128);
		self.FormatString(std::back_inserter(str));
		return str;
	}

private:
	static const std::string_view ms_names[64];
};

/**
 * std::formatter specialization to make ePermission formattable
 */
template<>
struct std::formatter<ePermission> {
	constexpr auto parse(std::format_parse_context& ctx) noexcept {
		return ctx.begin();
	}

	auto format(ePermission perm, std::format_context& ctx) const {
		return perm.FormatString(ctx.out());
	}
};

ePermission tag_invoke(boost::json::value_to_tag<ePermission>, const boost::json::value&);

constexpr ePermission ePermission::CreateInstantInvite              { 0x0000000000000001 }; // Allows creation of instant invites
constexpr ePermission ePermission::KickMembers                      { 0x0000000000000002 }; // Allows kicking members
constexpr ePermission ePermission::BanMembers                       { 0x0000000000000004 }; // Allows banning members
constexpr ePermission ePermission::Administrator                    { 0x0000000000000008 }; // Allows all permissions and bypasses channel permission overwrites
constexpr ePermission ePermission::ManageChannels                   { 0x0000000000000010 }; // Allows management and editing of channels
constexpr ePermission ePermission::ManageGuild                      { 0x0000000000000020 }; // Allows management and editing of the guild
constexpr ePermission ePermission::AddReactions                     { 0x0000000000000040 }; // Allows for adding new reactions to messages. This permission does not apply to reacting with an existing reaction on a message.
constexpr ePermission ePermission::ViewAuditLog                     { 0x0000000000000080 }; // Allows for viewing of audit logs
constexpr ePermission ePermission::PrioritySpeaker                  { 0x0000000000000100 }; // Allows for using priority speaker in a voice channel
constexpr ePermission ePermission::Stream                           { 0x0000000000000200 }; // Allows the user to go live
constexpr ePermission ePermission::ViewChannel                      { 0x0000000000000400 }; // Allows guild members to view a channel, which includes reading messages in text channels and joining voice channels
constexpr ePermission ePermission::SendMessages                     { 0x0000000000000800 }; // Allows for sending messages in a channel and creating threads in a forum (does not allow sending messages in threads)
constexpr ePermission ePermission::SendTTSMessages                  { 0x0000000000001000 }; // Allows for sending of /tts messages
constexpr ePermission ePermission::ManageMessages                   { 0x0000000000002000 }; // Allows for deletion of other users messages
constexpr ePermission ePermission::EmbedLinks                       { 0x0000000000004000 }; // Links sent by users with this permission will be auto-embedded
constexpr ePermission ePermission::AttachFiles                      { 0x0000000000008000 }; // Allows for uploading images and files
constexpr ePermission ePermission::ReadMessageHistory               { 0x0000000000010000 }; // Allows for reading of message history
constexpr ePermission ePermission::MentionEveryone                  { 0x0000000000020000 }; // Allows for using the @everyone tag to notify all users in a channel, and the @here tag to notify all online users in a channel
constexpr ePermission ePermission::UseExternalEmojis                { 0x0000000000040000 }; // Allows the usage of custom emojis from other servers
constexpr ePermission ePermission::ViewGuildInsights                { 0x0000000000080000 }; // Allows for viewing guild insights
constexpr ePermission ePermission::Connect                          { 0x0000000000100000 }; // Allows for joining of a voice channel
constexpr ePermission ePermission::Speak                            { 0x0000000000200000 }; // Allows for speaking in a voice channel
constexpr ePermission ePermission::MuteMembers                      { 0x0000000000400000 }; // Allows for muting members in a voice channel
constexpr ePermission ePermission::DeafenMembers                    { 0x0000000000800000 }; // Allows for deafening of members in a voice channel
constexpr ePermission ePermission::MoveMembers                      { 0x0000000001000000 }; // Allows for moving of members between voice channels
constexpr ePermission ePermission::UseVAD                           { 0x0000000002000000 }; // Allows for using voice-activity-detection in a voice channel
constexpr ePermission ePermission::ChangeNickname                   { 0x0000000004000000 }; // Allows for modification of own nickname
constexpr ePermission ePermission::ManageNicknames                  { 0x0000000008000000 }; // Allows for modification of other users nicknames
constexpr ePermission ePermission::ManageRoles                      { 0x0000000010000000 }; // Allows management and editing of roles
constexpr ePermission ePermission::ManageWebhooks                   { 0x0000000020000000 }; // Allows management and editing of webhooks
constexpr ePermission ePermission::ManageGuildExpressions           { 0x0000000040000000 }; // Allows for editing and deleting emojis, stickers, and soundboard sounds created by all users
constexpr ePermission ePermission::UseApplicationCommands           { 0x0000000080000000 }; // Allows members to use application commands, including slash commands and context menu commands.
constexpr ePermission ePermission::RequestToSpeak                   { 0x0000000100000000 }; // Allows for requesting to speak in stage channels. (This permission is under active development and may be changed or removed.)
constexpr ePermission ePermission::ManageEvents                     { 0x0000000200000000 }; // Allows for editing and deleting scheduled events created by all users
constexpr ePermission ePermission::ManageThreads                    { 0x0000000400000000 }; // Allows for deleting and archiving threads, and viewing all private threads
constexpr ePermission ePermission::CreatePublicThreads              { 0x0000000800000000 }; // Allows for creating public and announcement threads
constexpr ePermission ePermission::CreatePrivateThreads             { 0x0000001000000000 }; // Allows for creating private threads
constexpr ePermission ePermission::UseExternalStickers              { 0x0000002000000000 }; // Allows the usage of custom stickers from other servers
constexpr ePermission ePermission::SendMessagesInThreads            { 0x0000004000000000 }; // Allows for sending messages in threads
constexpr ePermission ePermission::UseEmbeddedActivities            { 0x0000008000000000 }; // Allows for using Activities (applications with the EMBEDDED flag)
constexpr ePermission ePermission::ModerateMembers                  { 0x0000010000000000 }; // Allows for timing out users to prevent them from sending or reacting to messages in chat and threads, and from speaking in voice and stage channels
constexpr ePermission ePermission::ViewCreatorMonetizationAnalytics { 0x0000020000000000 }; // Allows for viewing role subscription insights
constexpr ePermission ePermission::UseSoundboard                    { 0x0000040000000000 }; // Allows for using soundboard in a voice channel
constexpr ePermission ePermission::CreateGuildExpressions           { 0x0000080000000000 }; // Allows for creating emojis, stickers, and soundboard sounds, and editing and deleting those created by the current user.
constexpr ePermission ePermission::CreateEvents                     { 0x0000100000000000 }; // Allows for creating scheduled events, and editing and deleting those created by the current user.
constexpr ePermission ePermission::UseExternalSounds                { 0x0000200000000000 }; // Allows the usage of custom soundboard sounds from other servers
constexpr ePermission ePermission::SendVoiceMessages                { 0x0000400000000000 }; // Allows sending voice messages
/////////////////////////////////////////////////////////////////// { 0x0000800000000000 }; // Unused
/////////////////////////////////////////////////////////////////// { 0x0001000000000000 }; // Unused
constexpr ePermission ePermission::SendPolls                        { 0x0002000000000000 }; // Allows sending polls
constexpr ePermission ePermission::UseExternalApps                  { 0x0004000000000000 }; // Allows user-installed apps to send public responses. When disabled, users will still be allowed to use their apps but the responses will be ephemeral. This only applies to apps not also installed to the server.
constexpr ePermission ePermission::PinMessages                      { 0x0008000000000000 }; // Allows pinning and unpinning messages
constexpr ePermission ePermission::BypassSlowmode                   { 0x0010000000000000 }; // Allows bypassing slowmode restrictions
#endif //DISCORD_PERMISSION_H