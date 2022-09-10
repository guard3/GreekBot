#pragma once
#ifndef _GREEKBOT_GATEWAY_H_
#define _GREEKBOT_GATEWAY_H_
#include "Common.h"
#include "Task.h"
#include "User.h"
#include "Guild.h"
#include "Interaction.h"

enum eIntent {
	INTENT_GUILDS                    = 1 << 0,
	INTENT_GUILD_MEMBERS             = 1 << 1,
	INTENT_GUILD_BANS                = 1 << 2,
	INTENT_GUILD_EMOJIS_AND_STICKERS = 1 << 3,
	INTENT_GUILD_INTEGRATIONS        = 1 << 4,
	INTENT_GUILD_WEBHOOKS            = 1 << 5,
	INTENT_GUILD_INVITES             = 1 << 6,
	INTENT_GUILD_VOICE_STATES        = 1 << 7,
	INTENT_GUILD_PRESENCES           = 1 << 8,
	INTENT_GUILD_MESSAGES            = 1 << 9,
	INTENT_GUILD_MESSAGE_REACTIONS   = 1 << 10,
	INTENT_GUILD_MESSAGE_TYPING      = 1 << 11,
	INTENT_DIRECT_MESSAGES           = 1 << 12,
	INTENT_DIRECT_MESSAGE_REACTIONS  = 1 << 13,
	INTENT_DIRECT_MESSAGE_TYPING     = 1 << 14
};
inline eIntent operator|(eIntent a, eIntent b) { return (eIntent)((int)a | (int)b); }
inline eIntent operator&(eIntent a, eIntent b) { return (eIntent)((int)a & (int)b); }

class cHttpField final {
private:
	const char *m_name, *m_value;
public:
	cHttpField(const char* n, const char* v) : m_name(n), m_value(v) {}
	const char* GetName()  const noexcept { return m_name;  }
	const char* GetValue() const noexcept { return m_value; }
};

class cGateway {
private:
	class implementation;
	uhHandle<implementation> m_pImpl;

protected:
	const char* GetHttpAuthorization() const noexcept;

public:
	cGateway(const char* token, eIntent intents);
	cGateway(const cGateway&) = delete;
	cGateway(cGateway&&) noexcept = delete;
	~cGateway();

	cGateway& operator=(cGateway) = delete;

	const char* GetToken() const noexcept;

	cTask<json::value> DiscordGet   (const std::string& path, std::initializer_list<cHttpField> fields = {});
	cTask<json::value> DiscordPost  (const std::string& path, const json::object& obj, std::initializer_list<cHttpField> fields = {});
	cTask<json::value> DiscordPatch (const std::string& path, const json::object& obj, std::initializer_list<cHttpField> fields = {});
	cTask<json::value> DiscordPut   (const std::string& path, std::initializer_list<cHttpField> fields = {});
	cTask<json::value> DiscordPut   (const std::string& path, const json::object& obj, std::initializer_list<cHttpField> fields = {});
	cTask<json::value> DiscordDelete(const std::string& path, std::initializer_list<cHttpField> fields = {});

	cTask<> ResumeOnEventThread();

	virtual cTask<> OnReady(uhUser) { co_return; }
	virtual cTask<> OnGuildCreate(uhGuild) { co_return; }
	virtual cTask<> OnGuildRoleCreate(cSnowflake& guild_id, cRole& role) { co_return; }
	virtual cTask<> OnGuildRoleUpdate(cSnowflake& guild_id, cRole& role) { co_return; }
	virtual cTask<> OnGuildRoleDelete(cSnowflake& guild_id, cSnowflake& role_id) { co_return; }
	virtual cTask<> OnInteractionCreate(const cInteraction&) { co_return; }
	virtual cTask<> OnMessageCreate(cMessage&) { co_return; }

	void Run();
};
#endif /* _GREEKBOT_GATEWAY_H_ */