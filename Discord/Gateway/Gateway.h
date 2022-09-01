#pragma once
#ifndef _GREEKBOT_GATEWAY_H_
#define _GREEKBOT_GATEWAY_H_
#include "Common.h"
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

class cGateway {
private:
	class implementation;
	uhHandle<implementation> m_pImpl;

protected:
	const char* GetHttpAuthorization() const noexcept;

public:
	const char* GetToken() const noexcept;

	virtual void OnReady(uchUser) {}
	virtual void OnGuildCreate(uhGuild) {}
	virtual void OnGuildRoleCreate(chSnowflake guild_id, hRole role) {}
	virtual void OnGuildRoleUpdate(chSnowflake guild_id, hRole role) {}
	virtual void OnGuildRoleDelete(chSnowflake guild_id, chSnowflake role_id) {}
	virtual void OnInteractionCreate(chInteraction) {}
	virtual void OnMessageCreate(chMessage) {}

	cGateway(const char* token, eIntent intents);
	cGateway(const cGateway&) = delete;
	cGateway(cGateway&&) noexcept = delete;
	~cGateway();

	cGateway& operator=(cGateway) = delete;

	void Run();
};
#endif /* _GREEKBOT_GATEWAY_H_ */