#pragma once
#ifndef _GREEKBOT_GATEWAY_H_
#define _GREEKBOT_GATEWAY_H_
#include "Event.h"
#include "User.h"
#include "GatewayInfo.h"
#include <thread>
#include "Discord.h"

class cWebsocket;

class cGateway final {
private:
	char        m_http_auth[64];       // The authorization parameter for HTTP requests 'Bot token'
	const char *m_sessionId = nullptr; // The current session id, used for resuming; null = no valid session

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
	
	std::function<void(uchUser)> m_onReady;
	std::function<void(chInteraction)> m_onInteractionCreate;

	uchGatewayInfo get_gateway_info(cDiscordError&);
	
public:
	explicit cGateway(const char* token) { sprintf(m_http_auth, "Bot %.59s", token); }
	~cGateway() { delete[] m_sessionId; }
	
	template<typename F>
	cGateway& SetOnReady(F f) {
		m_onReady = f;
		return *this;
	}
	template<typename F>
	cGateway& SetOnInteractionCreate(F f) {
		m_onInteractionCreate = f;
		return *this;
	}
	
	void Run();
};


#endif /* _GREEKBOT_GATEWAY_H_*/
