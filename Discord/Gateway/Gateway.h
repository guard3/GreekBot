#pragma once
#ifndef _GREEKBOT_GATEWAY_H_
#define _GREEKBOT_GATEWAY_H_
#include "Event.h"
#include "User.h"
#include "GatewayInfo.h"
#include <thread>
#include "Discord.h"

class cWebsocket;

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
	char        m_http_auth[64];       // The authorization parameter for HTTP requests 'Bot token'
	const char *m_sessionId = nullptr; // The current session id, used for resuming; null = no valid session

	eIntent m_intents; // The gateway intents

	int m_last_sequence = 0; // The last event sequence received, used for heartbeating; 0 = none received
	
	/* Data relating to heartbeating */
	struct {
		std::thread thread;               // The heartbeating thread
		std::mutex  mutex;                // Mutex for accessing 'acknowledged'
		bool        should_exit  = true;  // Should the heartbeating thread exit?
		bool        acknowledged = false; // Is the heartbeat acknowledged?
	} m_heartbeat;
	bool SendHeartbeat();                 // Send a heartbeat to the gateway
	bool HeartbeatAcknowledged();         // Check if last heartbeat has been acknowledged by the gateway
	void AcknowledgeHeartbeat();          // Mark last heartbeat as acknowledged
	void StartHeartbeating(int interval); // Start sending heartbeats to the gateway every 'interval' milliseconds
	void StopHeartbeating();              // Stop sending heartbeats
	
	bool Identify();
	bool Resume();
	
	cWebsocket* m_pWebsocket = nullptr;
	
	bool OnEvent(chEvent event);

	uchGatewayInfo get_gateway_info(cDiscordError&);

protected:
	const char* GetHttpAuthorization() const { return m_http_auth;     }
	const char* GetToken()             const { return m_http_auth + 4; }

	virtual void OnReady(uchUser) {}
	virtual void OnGuildCreate(chGuild) {}
	virtual void OnInteractionCreate(chInteraction) {}
	virtual void OnMessageCreate(chMessage) {}
	
public:
	explicit cGateway(const char* token, eIntent intents) : m_intents(intents) { sprintf(m_http_auth, "Bot %.59s", token); }
	cGateway(const cGateway&) = delete;
	~cGateway() { delete[] m_sessionId; }
	
	void Run();
};


#endif /* _GREEKBOT_GATEWAY_H_*/
