#ifndef DISCORD_GATEWAY_H
#define DISCORD_GATEWAY_H
#include "Base.h"
#include "Coroutines.h"
#include "EmojiFwd.h"
#include "GatewayIntents.h"
#include "GuildFwd.h"
#include "InteractionFwd.h"
#include "MemberFwd.h"
#include "MessageFwd.h"
#include "RoleFwd.h"
#include "UserFwd.h"
#include "VoiceStateFwd.h"
#include <span>

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

struct cHttpField {
	std::string name, value;
};

class cGateway {
	struct implementation;
	std::unique_ptr<implementation> m_pImpl;
	/* A coroutine traits base type for void functions */
	template<typename...>
	friend struct coroutine_traits_base;

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
	virtual cTask<> OnHeartbeat() { co_return; }
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
};
#endif /* DISCORD_GATEWAY_H */