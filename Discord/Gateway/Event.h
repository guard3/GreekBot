#ifndef GREEKBOT_EVENT_H
#define GREEKBOT_EVENT_H
#include "json.h"
#include "User.h"
#include "Guild.h"
#include "Interaction.h"
#include "Message.h"
#include "GuildMembersResult.h"
#include "Application.h"

/* ================================================================================================================== */
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
	EVENT_GUILD_MEMBERS_CHUNK       = 0x343F4BC5,
	EVENT_USER_UPDATE               = 0xBFC98531
};
/* ================================================================================================================== */
namespace hidden {
	template<eEvent>
	class event_data;
/* ================================================================================================================== */
	template<>
	class event_data<EVENT_READY> final {
	public:
		int v;
		std::string session_id;
		std::string resume_gateway_url;
		uhUser user;
		cApplication application;

		event_data(const json::value& v): event_data(v.as_object()) {}
		event_data(const json::object& o):
			v(o.at("v").as_int64()),
			session_id(json::value_to<std::string>(o.at("session_id"))),
			resume_gateway_url(json::value_to<std::string>(o.at("resume_gateway_url"))),
			user(cHandle::MakeUnique<cUser>(o.at("user"))),
			application(o.at("application")) {}
	};
/* ================================================================================================================== */
	template<eEvent e> requires (e == EVENT_GUILD_ROLE_CREATE || e == EVENT_GUILD_ROLE_UPDATE)
	class event_data<e> final {
	public:
		cSnowflake guild_id;
		cRole      role;

		event_data(const json::object& o) : guild_id(o.at("guild_id")), role(o.at("role")) {}
		event_data(const json::value& v) : event_data(v.as_object()) {}
	};
/* ================================================================================================================== */
	template<>
	class event_data<EVENT_GUILD_ROLE_DELETE> final {
	public:
		cSnowflake guild_id, role_id;

		event_data(const json::object& o) : guild_id(o.at("guild_id")), role_id(o.at("role_id")) {}
		event_data(const json::value& v) : event_data(v.as_object()) {}
	};
/* ================================================================================================================== */
	template<eEvent e> struct map_event_data                            { typedef event_data<e>      type; };
	template<>         struct map_event_data<EVENT_GUILD_CREATE>        { typedef cGuild             type; };
	template<>         struct map_event_data<EVENT_INTERACTION_CREATE>  { typedef cInteraction       type; };
	template<>         struct map_event_data<EVENT_MESSAGE_CREATE>      { typedef cMessage           type; };
	template<>         struct map_event_data<EVENT_GUILD_MEMBERS_CHUNK> { typedef cGuildMembersChunk type; };
	template<>         struct map_event_data<EVENT_USER_UPDATE>         { typedef cUser              type; };
}
template<eEvent e> using tEventData = typename hidden::map_event_data<e>::type;

template<eEvent e> using   hEventData =   hHandle<tEventData<e>>;
template<eEvent e> using  chEventData =  chHandle<tEventData<e>>;
template<eEvent e> using  uhEventData =  uhHandle<tEventData<e>>;
template<eEvent e> using uchEventData = uchHandle<tEventData<e>>;
template<eEvent e> using  shEventData =  shHandle<tEventData<e>>;
template<eEvent e> using schEventData = schHandle<tEventData<e>>;

/* ================================================================================================================== */
class cEvent final {
private:
	std::string m_name; // Event name as a string
	eEvent      m_type; // Event type
	int64_t     m_seq;  // Event sequence - used for heartbeating
	json::value m_data; // Event specific data

public:
	cEvent(json::value&& v) : cEvent(v.as_object()) {}
	cEvent(json::object& o);
	
	eEvent             GetType()     const noexcept { return m_type; }
	const std::string& GetName()     const noexcept { return m_name; }
	int64_t            GetSequence() const noexcept { return m_seq;  }

	template<eEvent e>
	tEventData<e> GetData() const { return tEventData<e>(m_data); }

	template<eEvent e>
	uhEventData<e> GetDataPtr() const { return cHandle::MakeUnique<tEventData<e>>(m_data); }
};
typedef   hHandle<cEvent>   hEvent;
typedef  chHandle<cEvent>  chEvent;
typedef  uhHandle<cEvent>  uhEvent;
typedef uchHandle<cEvent> uchEvent;
typedef  shHandle<cEvent>  shEvent;
typedef schHandle<cEvent> schEvent;
#endif // GREEKBOT_EVENT_H