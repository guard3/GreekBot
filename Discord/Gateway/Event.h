#pragma once
#ifndef _GREEKBOT_EVENT_H_
#define _GREEKBOT_EVENT_H_
#include "json.h"
#include "User.h"
#include "Guild.h"
#include "Interaction.h"
#include "Message.h"

/* ================================================================================================= */
enum eEvent : uint32_t {
	EVENT_RESUMED                   = 0x712ED6EE,
	EVENT_READY                     = 0xDFC1471F,
	EVENT_GUILD_CREATE              = 0xDA68E698,
	EVENT_GUILD_MEMBER_UPDATE       = 0x73288C4D,
	EVENT_GUILD_ROLE_CREATE         = 0xEFECD22C,
	EVENT_GUILD_ROLE_UPDATE         = 0xF81F07AF,
	EVENT_GUILD_ROLE_DELETE         = 0x5A284E50,
	EVENT_GUILD_JOIN_REQUEST_DELETE = 0xC3A463EA,
	EVENT_INTERACTION_CREATE        = 0xB1E5D8EC,
	EVENT_MESSAGE_CREATE            = 0x9C643E55,
	EVENT_MESSAGE_UPDATE            = 0x8B97EBD6,
	EVENT_MESSAGE_DELETE            = 0x29A0A229,
};

/* ================================================================================================= */
class cReadyEventData final {
private:
	json::value m_data;

public:
	cReadyEventData(json::value v) : m_data(std::move(v)) {}

	// TODO: guilds, application
	int         GetVersion()   const { return m_data.at("v").as_int64();                     }
	const char* GetSessionId() const { return m_data.at("session_id").as_string().c_str();   }
	uchUser     GetUser()      const { return cHandle::MakeUnique<cUser>(m_data.at("user")); }
};
typedef   hHandle<cReadyEventData>   hReadyEventData;
typedef  chHandle<cReadyEventData>  chReadyEventData;
typedef  uhHandle<cReadyEventData>  uhReadyEventData;
typedef uchHandle<cReadyEventData> uchReadyEventData;
typedef  shHandle<cReadyEventData>  shReadyEventData;
typedef schHandle<cReadyEventData> schReadyEventData;

class cGuildRoleCreateUpdateEventData final {
private:
	cSnowflake guild_id;
	cRole      role;
public:
	cGuildRoleCreateUpdateEventData(const json::value& v) : cGuildRoleCreateUpdateEventData(v.as_object()) {}
	cGuildRoleCreateUpdateEventData(const json::object& o) : guild_id(o.at("guild_id")), role(o.at("role")) {}

	hSnowflake GetGuildId() { return &guild_id; }
	hRole      GetRole()    { return &role;     }
};
typedef   hHandle<cGuildRoleCreateUpdateEventData>   hGuildRoleCreateUpdateEventData;
typedef  chHandle<cGuildRoleCreateUpdateEventData>  chGuildRoleCreateUpdateEventData;
typedef  uhHandle<cGuildRoleCreateUpdateEventData>  uhGuildRoleCreateUpdateEventData;
typedef uchHandle<cGuildRoleCreateUpdateEventData> uchGuildRoleCreateUpdateEventData;
typedef  shHandle<cGuildRoleCreateUpdateEventData>  shGuildRoleCreateUpdateEventData;
typedef schHandle<cGuildRoleCreateUpdateEventData> schGuildRoleCreateUpdateEventData;

class cGuildRoleDeleteEventData final {
private:
	cSnowflake guild_id;
	cSnowflake role_id;

public:
	cGuildRoleDeleteEventData(const json::value& v) : cGuildRoleDeleteEventData(v.as_object()) {}
	cGuildRoleDeleteEventData(const json::object& o) : guild_id(o.at("guild_id")), role_id(o.at("role_id")) {}

	hSnowflake GetGuildId() { return &guild_id; }
	hSnowflake GetRoleId()  { return &role_id;  }
};
typedef   hHandle<cGuildRoleDeleteEventData>   hGuildRoleDeleteEventData;
typedef  chHandle<cGuildRoleDeleteEventData>  chGuildRoleDeleteEventData;
typedef  uhHandle<cGuildRoleDeleteEventData>  uhGuildRoleDeleteEventData;
typedef uchHandle<cGuildRoleDeleteEventData> uchGuildRoleDeleteEventData;
typedef  shHandle<cGuildRoleDeleteEventData>  shGuildRoleDeleteEventData;
typedef schHandle<cGuildRoleDeleteEventData> schGuildRoleDeleteEventData;

/* ================================================================================================= */
class cEvent final {
private:
	std::string m_name; // Event name as a string
	eEvent      m_type; // Event type
	int64_t     m_seq;  // Event sequence - used for heartbeating
	json::value m_data; // Event specific data

	template<eEvent> struct data_type {};
	template<> struct data_type<EVENT_READY>              { typedef cReadyEventData                 Type; };
	template<> struct data_type<EVENT_GUILD_CREATE>       { typedef cGuild                          Type; };
	template<> struct data_type<EVENT_GUILD_ROLE_CREATE>  { typedef cGuildRoleCreateUpdateEventData Type; };
	template<> struct data_type<EVENT_GUILD_ROLE_UPDATE>  { typedef cGuildRoleCreateUpdateEventData Type; };
	template<> struct data_type<EVENT_GUILD_ROLE_DELETE>  { typedef cGuildRoleDeleteEventData       Type; };
	template<> struct data_type<EVENT_INTERACTION_CREATE> { typedef cInteraction                    Type; };
	template<> struct data_type<EVENT_MESSAGE_CREATE>     { typedef cMessage                        Type; };
	template<eEvent e> using tDataType = typename data_type<e>::Type;

public:
	cEvent(json::value&& v) : cEvent(v.as_object()) {}
	cEvent(json::object& o);
	
	eEvent      GetType()     const { return m_type;         }
	const char* GetName()     const { return m_name.c_str(); }
	int64_t     GetSequence() const { return m_seq;          }

	template<eEvent e>
	uhHandle<tDataType<e>> GetData() const { return cHandle::MakeUniqueNoEx<tDataType<e>>(m_data); }
};
typedef   hHandle<cEvent>   hEvent;
typedef  chHandle<cEvent>  chEvent;
typedef  uhHandle<cEvent>  uhEvent;
typedef uchHandle<cEvent> uchEvent;
typedef  shHandle<cEvent>  shEvent;
typedef schHandle<cEvent> schEvent;
#endif /* _GREEKBOT_EVENT_H_ */