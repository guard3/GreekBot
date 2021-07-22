#pragma once
#ifndef _GREEKBOT_EVENT_H_
#define _GREEKBOT_EVENT_H_
#include "json.h"

enum eEvent {
	// 0: Generic event
	// 1: Heartbeat command
	EVENT_HELLO = 10,
	EVENT_HEARTBEAT_ACK = 11,
	EVENT_READY,
	EVENT_NOT_IMPLEMENTED
};

class cHelloEvent final {
private:
	int heartbeat_interval;

public:
	cHelloEvent(const json::value& v) : heartbeat_interval(static_cast<int>(v.at("heartbeat_interval").as_int64())) {}
	
	int GetHeartbeatInterval() const { return heartbeat_interval; }
};
typedef const std::unique_ptr<cHelloEvent> hHelloEvent;

class cReadyEvent final {
private:
	int v;
	// TODO: user
	// TODO: guilds
	const json::string session_id;
	// TODO: application
public:
	cReadyEvent(const json::value& v) : v(static_cast<int>(v.at("v").as_int64())), session_id(v.at("session_id").as_string()) {}
	
	int         GetVersion()   { return v;                  }
	const char* GetSessionId() { return session_id.c_str(); }
};
typedef const std::unique_ptr<cReadyEvent> hReadyEvent;

class cEvent final {
private:
	eEvent            t; // Event type
	int               s; // Event sequence - used for heartbeating
	const json::value d; // Event specific data
	
public:
	cEvent(const json::value& v);
	
	eEvent GetType()     const { return t; }
	int    GetSequence() const { return s; }
	
	hHelloEvent GetHelloData() const {
		try {
			return std::make_unique<cHelloEvent>(d);
		}
		catch (const std::exception&) {
			return std::unique_ptr<cHelloEvent>();
		}
	}
	hReadyEvent GetReadyData() const {
		try {
			return std::make_unique<cReadyEvent>(d);
		}
		catch (const std::exception&) {
			return std::unique_ptr<cReadyEvent>();
		}
	}
};
typedef const std::unique_ptr<cEvent> hEvent;

#endif /* _GREEKBOT_EVENT_H_ */
