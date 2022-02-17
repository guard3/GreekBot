#pragma once
#ifndef _GREEKBOT_GATEWAY_H_
#define _GREEKBOT_GATEWAY_H_
#include "Utils.h"
#include "json.h"

#include <thread>
#include <atomic>

#include "Types.h"
#include "User.h"
#include "Guild.h"
#include "GatewayInfo.h"
#include "Discord.h"

class cGatewaySession;

class cUser;
typedef uchHandle<cUser> uchUser;
class cInteraction;
typedef chHandle<cInteraction> chInteraction;
class cMessage;
typedef chHandle<cMessage> chMessage;

class cEvent;
class cGatewayInfo;
typedef uchHandle<cGatewayInfo> uchGatewayInfo;

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
	/* Initial parameters for identifying */
	std::string m_http_auth; // The authorization parameter for HTTP requests 'Bot token'
	eIntent     m_intents;   // The gateway intents
	/* Session attributes */
	std::string         m_session_id;    // The current session id, used for resuming; empty = no valid session
	std::atomic_int64_t m_last_sequence; // The last event sequence received, used for heartbeating; 0 = none received
	/* Heartbeating */
	std::thread      m_heartbeat_thread; // The heartbeating thread
	std::atomic_bool m_heartbeat_exit;   // Should the heartbeating thread exit?
	std::atomic_bool m_heartbeat_ack;    // Is the heartbeat acknowledged?
	/* Json parsing for gateway events */
	json::monotonic_resource m_mr;          // A monotonic memory resource for json parsing
	json::stream_parser      m_json_parser; // The json parser
	/* Websocket session */
	cGatewaySession* m_session;
	/* Session close reason */
	int         m_close_code;
	std::string m_close_msg;
	/* Gateway commands */
	void resume();
	void identify();
	void heartbeat();
	void start_heartbeating(int64_t);
	void stop_heartbeating();
	/* Websocket message queuing */
	void send(std::string);
	/* Beast/Asio async functions */
	void on_read(boost::system::error_code, size_t);
	void on_write(boost::system::error_code, size_t);
	/* A method that initiates the gateway connection */
	void run_session(const char* url);
	uchGatewayInfo get_gateway_info(cDiscordError&);
	/* A method that's invoked for every gateway event */
	void on_event(const cEvent& event);

protected:
	const char* GetHttpAuthorization() const { return m_http_auth.c_str(); }

public:
	const char* GetToken() const { return m_http_auth.c_str() + 4; }

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