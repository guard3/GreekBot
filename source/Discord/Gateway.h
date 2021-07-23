#pragma once
#ifndef _GREEKBOT_GATEWAY_H_
#define _GREEKBOT_GATEWAY_H_
#include "JsonError.h"
#include "Websocket.h"
#include "Event.h"
#include <thread>

class cSessionStartLimit final {
private:
	int m_values[4];
	
public:
	/* Construct a session_start_object from a gateway JSON - throws exceptions */
	cSessionStartLimit(const json::object& obj) : m_values {
		static_cast<int>(obj.at("total"          ).as_int64()),
		static_cast<int>(obj.at("remaining"      ).as_int64()),
		static_cast<int>(obj.at("reset_after"    ).as_int64()),
		static_cast<int>(obj.at("max_concurrency").as_int64())
	} {}
	
	/* Attributes */
	int GetTotal()          const { return m_values[0]; }
	int GetRemaining()      const { return m_values[1]; }
	int GetResetAfter()     const { return m_values[2]; }
	int GetMaxConcurrency() const { return m_values[3]; }
};
typedef const cSessionStartLimit* hSessionStartLimit;

class cGatewayInfo final {
private:
	hJsonError         m_err    = nullptr;
	hSessionStartLimit m_ssl    = nullptr;
	char*              m_url    = nullptr;
	int                m_shards = 0;
	
public:
	/* Get gateway info - throws exceptions */
	cGatewayInfo(const char* auth);
	
	hJsonError         GetError()             const { return m_err;    }
	hSessionStartLimit GetSessionStartLimit() const { return m_ssl;    }
	const char*        GetUrl()               const { return m_url;    }
	int                GetShards()            const { return m_shards; }
	
};
typedef const std::unique_ptr<cGatewayInfo> hGatewayInfo;

class cGateway final {
private:
	char m_token[60];              // The authentication token
	char m_sessionId[40] { '\0' }; // The current session id; used for resuming
	
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
	
	static hGatewayInfo GetGatewayInfo(const char* auth) {
		try {
			return std::make_unique<cGatewayInfo>(auth);
		}
		catch (const std::exception& e) {
			return std::unique_ptr<cGatewayInfo>();
		}
	}
	
	hEvent GetEvent();
	
	cWebsocket m_ws;
	
	void OnConnect();
	
public:
	cGateway(const char* token);
	~cGateway();
	
	void Run(const char* auth);
};


#endif /* _GREEKBOT_GATEWAY_H_*/
