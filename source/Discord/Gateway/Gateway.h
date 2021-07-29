#pragma once
#ifndef _GREEKBOT_GATEWAY_H_
#define _GREEKBOT_GATEWAY_H_
#include "Event.h"
#include "User.h"
#include <thread>

class cWebsocket;

class cGateway final {
private:
	char  m_http_auth[64] = "Bot ";          // The authorization paramerer for HTTP requests 'Bot token'
	char  m_sessionId[40] = "";              // The current session id; used for resuming
	char *m_token         = m_http_auth + 4; // The authentication token
	
	/* Sequence */
	struct {
		std::mutex mutex;
		int value = 0;      // The last event sequence received; 0 = none received
	} m_last_sequence;
	int  GetLastSequence();
	void SetLastSequence(int);
	
	void ResetSession();
	
	/* Data relating to heartbeating */
	struct {
		std::thread thread;       // The heartbeating thread
		std::mutex  mutex;        // Mutex for accessing 'acknowledged'
		bool        should_exit;  // Should the heartbeating thread exit?
		bool        acknowledged; // Is the heartbeat acknowledged?
	} m_heartbeat;
	bool SendHeartbeat();                 // Send a heartbeat to the gateway
	bool HeartbeatAcknowledged();         // Check if last heartbeat has been acknowledged by the gateway
	void AcknowledgeHeartbeat();          // Mark last heartbeat as acknowledged
	void StartHeartbeating(int interval); // Start sending heartbeats to the gateway every 'interval' milliseconds
	void StopHeartbeating();              // Stop sending heartbeats
	
	
	bool Identify();
	bool Resume();
	
	cWebsocket* m_pWebsocket;
	
	bool OnEvent(cEvent* event);
	
	std::function<void(uchUser)> m_onReady;
	std::function<void(uchInteraction)> m_onInteractionCreate;
	
public:
	cGateway(const char* token);
	~cGateway();
	
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
