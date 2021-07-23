#pragma once
#ifndef _GREEKBOT_GATEWAY_H_
#define _GREEKBOT_GATEWAY_H_
#include "Event.h"
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
	
	/* Data relating to heartbeating */
	struct {
		std::thread thread;       // The heartbeating thread
		std::mutex  mutex;        // Mutex for accessing 'acknowledged'
		bool        acknowledged; // Is the heartbeat acknowledged?
	} m_heartbeat;
	bool SendHeartbeat();                 // Send a heartbeat to the gateway
	bool HeartbeatAcknowledged();         // Check if last heartbeat has been acknowledged by the gateway
	void AcknowledgeHeartbeat();          // Mark last heartbeat as acknowledged
	void StartHeartbeating(int interval); // Start sending heartbeats to the gateway every 'interval' milliseconds
	
	
	bool Identify();
	
	hEvent GetEvent();
	
	cWebsocket* m_pWebsocket;
	
	void OnEvent(cEvent* event);
	
	void OnConnect();
	
public:
	cGateway(const char* token);
	~cGateway();
	
	void Run();
};


#endif /* _GREEKBOT_GATEWAY_H_*/
