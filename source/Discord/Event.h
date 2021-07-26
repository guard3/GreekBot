#pragma once
#ifndef _GREEKBOT_EVENT_H_
#define _GREEKBOT_EVENT_H_
#include "json.h"
#include "User.h"

enum eEvent {
	/* Payload opcodes */
	// 0: Dispatch
	EVENT_HEARTBEAT       = 1,
	// 2: Identify
	// 3: Presence update
	// 4: Voice state update
	// 5: -
	// 6: Resume
	EVENT_RECONNECT       = 7,
	// 8: Request quild members
	EVENT_INVALID_SESSION = 9,
	EVENT_HELLO           = 10,
	EVENT_HEARTBEAT_ACK   = 11,
	
	/* Events */
	EVENT_READY,
	EVENT_NOT_IMPLEMENTED
};

/* INVALID_SESSION data */
class cInvalidSessionEvent final {
private:
	bool resumable;
	
public:
	cInvalidSessionEvent(const json::value& v) : resumable(v.as_bool()) {}
	
	bool IsSessionResumable() const { return resumable; }
};
typedef const std::unique_ptr<cInvalidSessionEvent> hInvalidSessionEvent;

/* HELLO data */
class cHelloEvent final {
private:
	int heartbeat_interval;

public:
	cHelloEvent(const json::value& v) : heartbeat_interval(static_cast<int>(v.at("heartbeat_interval").as_int64())) {}
	
	int GetHeartbeatInterval() const { return heartbeat_interval; }
};
typedef const std::unique_ptr<cHelloEvent> hHelloEvent;

/* READY data */
class cReadyEvent final {
private:
	int v;
	hUser user;
	// TODO: guilds
	const json::string session_id;
	// TODO: application
public:
	cReadyEvent(const json::value& v) : v(static_cast<int>(v.at("v").as_int64())), user(new cUser(v.at("user"))), session_id(v.at("session_id").as_string()) {}
	
	int         GetVersion()   const { return v;                  }
	hUser       GetUser()      const { return user;               }
	const char* GetSessionId() const { return session_id.c_str(); }
};
typedef const std::unique_ptr<cReadyEvent> hReadyEvent;

/* Matching event type to event data */
template<eEvent event> struct tEventType {};
template<> struct tEventType<EVENT_INVALID_SESSION> { typedef cInvalidSessionEvent Type; };
template<> struct tEventType<EVENT_HELLO>           { typedef cHelloEvent          Type; };
template<> struct tEventType<EVENT_READY>           { typedef cReadyEvent          Type; };

class cEvent final {
private:
	eEvent            t; // Event type
	int               s; // Event sequence - used for heartbeating
	const json::value d; // Event specific data
	
public:
	cEvent(const json::value& v);
	
	eEvent GetType()     const { return t; }
	int    GetSequence() const { return s; }
	
	template<eEvent event>
	const auto GetData() const {
		try {
			return std::make_unique<typename tEventType<event>::Type>(d);
		}
		catch (const std::exception&) {
			return std::unique_ptr<typename tEventType<event>::Type>();
		}
	}
};
typedef const std::unique_ptr<cEvent> hEvent;

#endif /* _GREEKBOT_EVENT_H_ */
