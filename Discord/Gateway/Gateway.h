#ifndef DISCORD_GATEWAY_H
#define DISCORD_GATEWAY_H
#include "Coroutines.h"
#include "EmojiFwd.h"
#include "Guild.h"
#include "InteractionFwd.h"
#include "Member.h"
#include "MessageFwd.h"
#include "VoiceState.h"

enum eIntent {
	INTENT_GUILDS                        = 1 << 0,
	INTENT_GUILD_MEMBERS                 = 1 << 1,
	INTENT_GUILD_MODERATION              = 1 << 2,
	INTENT_GUILD_EMOJIS_AND_STICKERS     = 1 << 3,
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
inline eIntent operator|(eIntent a, eIntent b) { return (eIntent)((int)a | (int)b); }
inline eIntent operator&(eIntent a, eIntent b) { return (eIntent)((int)a & (int)b); }

struct xGatewayError : std::runtime_error {
	xGatewayError(const char* what_arg) : std::runtime_error(what_arg) {}
};
struct xGatewaySessionResetError : xGatewayError {
	xGatewaySessionResetError() : xGatewayError("The request can't be fulfilled because the Gateway session was reset.") {}
};
struct xGatewayEventError : xGatewayError {
	xGatewayEventError() : xGatewayError("The event parameters were invalid.") {}
};
struct xGatewayTimeoutError : xGatewayError {
	xGatewayTimeoutError() : xGatewayError("The request timed out.") {}
};
struct xGatewayPrivilegedIntentsError : xGatewayError {
	xGatewayPrivilegedIntentsError() : xGatewayError("The request can't be fulfilled because you are missing privileged intents.") {}
};

class cHttpField final {
private:
	std::string m_name, m_value;
public:
	cHttpField(std::string n, std::string v) : m_name(std::move(n)), m_value(std::move(v)) {}
	std::string_view GetName()  const noexcept { return m_name;  }
	std::string_view GetValue() const noexcept { return m_value; }
};
typedef std::vector<cHttpField> tHttpFields;

class cGateway {
	class implementation;
	std::unique_ptr<implementation> m_pImpl;

protected:
	std::string_view GetHttpAuthorization() const noexcept;

public:
	cGateway(std::string_view token, eIntent intents);
	cGateway(const cGateway&) = delete;
	~cGateway();

	cGateway& operator=(const cGateway&) = delete;

	std::string_view GetToken() const noexcept;

	cTask<boost::json::value> DiscordGet   (std::string_view path,                                 std::span<const cHttpField> fields = {});
	cTask<boost::json::value> DiscordPost  (std::string_view path, const boost::json::object& obj, std::span<const cHttpField> fields = {});
	cTask<boost::json::value> DiscordPatch (std::string_view path, const boost::json::object& obj, std::span<const cHttpField> fields = {});
	cTask<boost::json::value> DiscordPut   (std::string_view path,                                 std::span<const cHttpField> fields = {});
	cTask<boost::json::value> DiscordPut   (std::string_view path, const boost::json::object& obj, std::span<const cHttpField> fields = {});
	cTask<boost::json::value> DiscordDelete(std::string_view path,                                 std::span<const cHttpField> fields = {});

	cTask<boost::json::value> DiscordPostNoRetry(std::string_view path, const boost::json::object& obj, std::span<const cHttpField> fields = {});

	cTask<> ResumeOnEventStrand();
	cTask<> WaitOnEventStrand(std::chrono::milliseconds);

	cAsyncGenerator<cMember> RequestGuildMembers(const cSnowflake& guild_id, std::string_view query = {});
	cAsyncGenerator<cMember> RequestGuildMembers(const cSnowflake& guild_id, std::span<const cSnowflake> user_ids);

	virtual cTask<> OnReady(cUser&) { co_return; }
	virtual cTask<> OnUserUpdate(cUser&) { co_return; }
	virtual cTask<> OnGuildCreate(cGuild&, cGuildCreate&) { co_return; }
	virtual cTask<> OnGuildRoleCreate(cSnowflake& guild_id, cRole& role) { co_return; }
	virtual cTask<> OnGuildRoleUpdate(cSnowflake& guild_id, cRole& role) { co_return; }
	virtual cTask<> OnGuildRoleDelete(cSnowflake& guild_id, cSnowflake& role_id) { co_return; }
	virtual cTask<> OnGuildMemberAdd(cSnowflake& guild_id, cMember& member) { co_return; }
	virtual cTask<> OnGuildMemberUpdate(cSnowflake& guild_id, cMemberUpdate& member) { co_return; }
	virtual cTask<> OnGuildMemberRemove(cSnowflake& guild_id, cUser& user) { co_return; }
	virtual cTask<> OnInteractionCreate(cInteraction&) { co_return; }
	virtual cTask<> OnMessageCreate(cMessage& msg, hSnowflake guild_id, hPartialMember member) { co_return; }
	virtual cTask<> OnMessageUpdate(cMessageUpdate& msg, hSnowflake guild_id, hPartialMember member) { co_return; }
	virtual cTask<> OnMessageDelete(cSnowflake& id, cSnowflake& channel_id, hSnowflake guild_id) { co_return; }
	virtual cTask<> OnMessageDeleteBulk(std::span<cSnowflake> ids, cSnowflake& channel_id, hSnowflake guild_id) { co_return; }
	virtual cTask<> OnMessageReactionAdd(cSnowflake& user_id, cSnowflake& channel_id, cSnowflake& message_id, hSnowflake guild_id, hSnowflake message_author_id, hMember member, cEmoji& emoji) { co_return; }
	virtual cTask<> OnMessageReactionRemove(cSnowflake& user_id, cSnowflake& channel_id, cSnowflake& message_id, hSnowflake guild_id, cEmoji& emoji) { co_return; }
	virtual cTask<> OnMessageReactionRemoveAll(cSnowflake& channel_id, cSnowflake& message_id, hSnowflake guild_id) { co_return; }
	virtual cTask<> OnMessageReactionRemoveEmoji(cSnowflake& channel_id, cSnowflake& message_id, hSnowflake guild_id, cEmoji&) { co_return; }
	virtual cTask<> OnVoiceStateUpdate(cVoiceState&) { co_return; }
	/* Run the client loop */
	void Run();
	/* Gracefully close the client connection and request program exit */
	void RequestExit() noexcept;
};
#endif /* DISCORD_GATEWAY_H */