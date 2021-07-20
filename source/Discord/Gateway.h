#pragma once
#ifndef _GREEKBOT_GATEWAY_H_
#define _GREEKBOT_GATEWAY_H_
#include "JsonError.h"
#include "Websocket.h"
#include "Payload.h"
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

class cGateway final : public cWebsocket {
private:
	std::thread m_heartbeatThread;
	
	static hGatewayInfo GetGatewayInfo(const char* auth) {
		try {
			return std::make_unique<cGatewayInfo>(auth);
		}
		catch (const std::exception& e) {
			return std::unique_ptr<cGatewayInfo>();
		}
	}
	
	hPayload ReceivePayload();
	void StartHeartbeating(int interval);
	
public:
	cGateway() : cWebsocket() {}
	
	void OnHandshake() override;
	
	void Run(const char* auth);
};


#endif /* _GREEKBOT_GATEWAY_H_*/
