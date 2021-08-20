#pragma once
#ifndef _GREEKBOT_EVENT_H_
#define _GREEKBOT_EVENT_H_
#include "json.h"
#include "User.h"
#include "Interaction.h"

/* ================================================================================================= */
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
	EVENT_INTERACTION_CREATE,
	EVENT_NOT_IMPLEMENTED
};

/* ================================================================================================= */
class cReadyEventData final {
private:
	int         version;
	json::value user;
	// TODO: guilds
	std::string session_id;
	// TODO: application
public:
	cReadyEventData(const json::value& v) : version(static_cast<int>(v.at("v").as_int64())), user(v.at("user")), session_id(v.at("session_id").as_string().c_str()) {}
	
	int         GetVersion()   const { return version;            }
	const char* GetSessionId() const { return session_id.c_str(); }
	
	uchUser GetUser() const {
		try {
			return std::make_unique<const cUser>(user);
		}
		catch (const std::exception&) {
			return uchUser();
		}
	}
};
typedef   hHandle<cReadyEventData>   hReadyEventData;
typedef  chHandle<cReadyEventData>  chReadyEventData;
typedef  uhHandle<cReadyEventData>  uhReadyEventData;
typedef uchHandle<cReadyEventData> uchReadyEventData;
typedef  shHandle<cReadyEventData>  shReadyEventData;
typedef schHandle<cReadyEventData> schReadyEventData;

/* ================================================================================================= */
class cEvent final {
private:
	eEvent      t; // Event type
	int         s; // Event sequence - used for heartbeating
	json::value d; // Event specific data
	
	template<eEvent> struct _t {};
	template<> struct _t<EVENT_INVALID_SESSION>    { typedef bool              Type; };
	template<> struct _t<EVENT_HELLO>              { typedef int               Type; };
	template<> struct _t<EVENT_READY>              { typedef uchReadyEventData Type; };
	template<> struct _t<EVENT_INTERACTION_CREATE> { typedef uchInteraction    Type; };
	
	template<eEvent e> using tEventType = typename _t<e>::Type;
	
	template<typename> struct _r {};
	template<typename T> struct _r<uchHandle<T>> { typedef T Type; };
	
	template<typename T> using remove_uch = typename _r<T>::Type;
	
public:
	cEvent(const json::value& v);
	
	eEvent GetType()     const { return t; }
	int    GetSequence() const { return s; }
	
	template<eEvent e>
	tEventType<e> GetData() const {
		typedef tEventType<e> return_t;
		try {
			return std::make_unique<remove_uch<return_t>>(d);
		}
		catch (const std::exception&) {
			return return_t();
		}
	}
	
	template<>
	tEventType<EVENT_INVALID_SESSION> GetData<EVENT_INVALID_SESSION>() const {
		auto p = d.if_bool();
		return p && t == EVENT_INVALID_SESSION ? *p : false;
	}
	
	template<>
	tEventType<EVENT_HELLO> GetData<EVENT_HELLO>() const {
		if (auto o = d.if_object()) {
			if (auto v = o->if_contains("heartbeat_interval")) {
				auto p = v->if_int64();
				if (p && t == EVENT_HELLO)
					return static_cast<int>(*p);
			}
		}
		return -1;
	}
};
typedef   hHandle<cEvent>   hEvent;
typedef  chHandle<cEvent>  chEvent;
typedef  uhHandle<cEvent>  uhEvent;
typedef uchHandle<cEvent> uchEvent;
typedef  shHandle<cEvent>  shEvent;
typedef schHandle<cEvent> schEvent;

#endif /* _GREEKBOT_EVENT_H_ */