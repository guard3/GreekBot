#ifndef DISCORD_GATEWAYINTENTS_H
#define DISCORD_GATEWAYINTENTS_H
#include <utility>
enum eIntent {
	INTENT_GUILDS                        = 1 << 0,
	INTENT_GUILD_MEMBERS                 = 1 << 1,
	INTENT_GUILD_MODERATION              = 1 << 2,
	INTENT_GUILD_EXPRESSIONS             = 1 << 3,
	INTENT_GUILD_INTEGRATIONS            = 1 << 4,
	INTENT_GUILD_WEBHOOKS                = 1 << 5,
	INTENT_GUILD_INVITES                 = 1 << 6,
	INTENT_GUILD_VOICE_STATES            = 1 << 7,
	INTENT_GUILD_PRESENCES               = 1 << 8,
	INTENT_GUILD_MESSAGES                = 1 << 9,
	INTENT_GUILD_MESSAGE_REACTIONS       = 1 << 10,
	INTENT_GUILD_MESSAGE_TYPING          = 1 << 11,
	INTENT_DIRECT_MESSAGES               = 1 << 12,
	INTENT_DIRECT_MESSAGE_REACTIONS      = 1 << 13,
	INTENT_DIRECT_MESSAGE_TYPING         = 1 << 14,
	INTENT_MESSAGE_CONTENT               = 1 << 15,
	INTENT_GUILD_SCHEDULED_EVENTS        = 1 << 16,
	INTENT_AUTO_MODERATION_CONFIGURATION = 1 << 20,
	INTENT_AUTO_MODERATION_EXECUTION     = 1 << 21,
	INTENT_GUILD_MESSAGE_POLLS           = 1 << 24,
	INTENT_DIRECT_MESSAGE_POLLS          = 1 << 25,
};
inline constexpr eIntent operator|(eIntent lhs, eIntent rhs) { return static_cast<eIntent>(std::to_underlying(lhs) | std::to_underlying(rhs)); }
inline constexpr eIntent operator&(eIntent lhs, eIntent rhs) { return static_cast<eIntent>(std::to_underlying(lhs) | std::to_underlying(rhs)); }
#endif /* DISCORD_GATEWAYINTENTS_H */
