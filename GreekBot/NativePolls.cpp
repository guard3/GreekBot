#include "GreekBot.h"
#include "Utils.h"

cTask<>
cGreekBot::process_native_polls_reaction_add(crefUser user, crefChannel channel, crefMessage message, hMember member, const cEmoji& emoji) HANDLER_BEGIN {
	// Check that we're in the native polls channel
	if (channel.GetId() != LMG_CHANNEL_NATIVE_POLLS)
		co_return;

	// Make sure that we have a member object available to check the roles
	std::optional<cMember> mem;
	if (!member) {
		member = &mem.emplace(co_await GetGuildMember(LMG_GUILD_ID, user));
	}

	// Only Natives should be allowed to react...
	if (std::ranges::contains(member->GetRoles(), LMG_ROLE_NATIVE))
		co_return;

	// ...otherwise, remove the reaction and give them a heads-up in DMs
	co_await DeleteReaction(channel, message, emoji, user);
	try {
		co_await CreateDMMessage(user, cPartialMessage().SetContent(std::format(
			"Your reaction in {:c} was removed {}\n"
			"This channel is meant to give insight into how common different variants of a given word are, so only native input counts.", channel.GetId(), LMG_EMOJI_HEBONK))
		);
	} catch (const xDiscordError& ex) {
		cUtils::PrintLog("{} with id {} posted a reaction in native polls and couldn't be reached in DMs about it: {}", member->GetUser() ? member->GetUser()->GetUsername() : "User", user.GetId(), ex.what());
	}
} HANDLER_END
