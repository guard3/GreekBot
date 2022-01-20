#pragma once
#ifndef _GREEKBOT_EVENT_H_
#define _GREEKBOT_EVENT_H_
#include "json.h"
#include "User.h"
#include "Guild.h"
#include "Interaction.h"
#include "Message.h"

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
	// 8: Request guild members
	EVENT_INVALID_SESSION = 9,
	EVENT_HELLO           = 10,
	EVENT_HEARTBEAT_ACK   = 11,
	
	/* Events */
	EVENT_READY,
	EVENT_GUILD_CREATE,
	EVENT_INTERACTION_CREATE,
	EVENT_MESSAGE_CREATE,
	EVENT_NOT_IMPLEMENTED
};

/* ================================================================================================= */
class cReadyEventData final {
private:
	json::value m_value;

public:
	explicit cReadyEventData(json::value v) : m_value(std::move(v)) {}

	// TODO: guilds, application
	[[nodiscard]] int GetVersion() const {
		try {
			return static_cast<int>(m_value.at("v").as_int64());
		}
		catch (...) {
			return 0;
		}
	}

	[[nodiscard]] uchHandle<char[]> GetSessionId() const {
		try {
			auto& s = m_value.at("session_id").as_string();
			auto result = cHandle::MakeUnique<char[]>(s.size() + 1);
			strcpy(result.get(), s.c_str());
			return result;
		}
		catch (...) {
			return {};
		}
	}

	[[nodiscard]] uchUser GetUser() const {
		try {
			return cHandle::MakeUnique<cUser>(m_value.at("user"));
		}
		catch (...) {
			return {};
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

	template<eEvent> struct data_type {};
	template<> struct data_type<EVENT_READY>              { typedef cReadyEventData Type; };
	template<> struct data_type<EVENT_GUILD_CREATE>       { typedef cGuild          Type; };
	template<> struct data_type<EVENT_INTERACTION_CREATE> { typedef cInteraction    Type; };
	template<> struct data_type<EVENT_MESSAGE_CREATE>     { typedef cMessage        Type; };
	template<eEvent e> using tDataType = typename data_type<e>::Type;

	template<eEvent e> struct return_type { typedef uchHandle<tDataType<e>> Type; };
	template<> struct return_type<EVENT_INVALID_SESSION> { typedef bool Type; };
	template<> struct return_type<EVENT_HELLO>           { typedef int  Type; };
	template<eEvent e> using tReturnType = typename return_type<e>::Type;

public:
	cEvent(const json::value& v);
	
	eEvent GetType()     const { return t; }
	int    GetSequence() const { return s; }

	template<eEvent e>
	tReturnType<e> GetData() const {
		if constexpr (e == EVENT_INVALID_SESSION) {
			auto p = d.if_bool();
			return p && *p && t == EVENT_INVALID_SESSION;
		}
		else if constexpr(e == EVENT_HELLO) {
			try {
				return d.at("heartbeat_interval").as_int64();
			}
			catch (...) {
				return -1;
			}
		}
		else {
			return cHandle::MakeUniqueConstNoEx<tDataType<e>>(d);
		}
	}
};
typedef   hHandle<cEvent>   hEvent;
typedef  chHandle<cEvent>  chEvent;
typedef  uhHandle<cEvent>  uhEvent;
typedef uchHandle<cEvent> uchEvent;
typedef  shHandle<cEvent>  shEvent;
typedef schHandle<cEvent> schEvent;

#endif /* _GREEKBOT_EVENT_H_ */
