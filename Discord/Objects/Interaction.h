#pragma once
#ifndef _GREEKBOT_INTERACTION_H_
#define _GREEKBOT_INTERACTION_H_
#include "Types.h"
#include "json.h"
#include "User.h"
#include "Member.h"
#include <vector>

/* ================================================================================================= */
enum eInteractionType {
	INTERACTION_PING = 1,
	INTERACTION_APPLICATION_COMMAND,
	INTERACTION_MESSAGE_COMPONENT
};

/* ================================================================================================= */
enum eApplicationCommandType {
	APP_COMMAND_SUB_COMMAND = 1,
	APP_COMMAND_SUB_COMMAND_GROUP,
	APP_COMMAND_STRING,
	APP_COMMAND_INTEGER,
	APP_COMMAND_BOOLEAN,
	APP_COMMAND_USER,
	APP_COMMAND_CHANNEL,
	APP_COMMAND_ROLE,
	APP_COMMAND_MENTIONABLE,
	APP_COMMAND_NUMBER
};

/* ================================================================================================= */
class cInteractionDataResolved final {
private:
	template<typename T>
	class cMap final {
	private:
		const json::value m_value;
		
	public:
		cMap() {}
		cMap(const json::value& v) : m_value(v) {}
		
		uchHandle<T> operator[](const char* snowflake) const {
			try {
				return std::make_unique<const T>(m_value.at(snowflake));
			}
			catch (const std::exception&) {
				return uchHandle<T>();
			}
		}
		
		auto operator[](const cSnowflake& snowflake) const {
			return operator[](snowflake.ToString());
		}
		
		template<typename S>
		auto operator[](S snowflake) const {
			return operator[](snowflake->ToString());
		}
	};
	typedef cMap<cUser>   tUserMap;
	typedef cMap<cMember> tMemberMap;
	
public:
	const tUserMap   Users;
	const tMemberMap Members;
	// roles
	// channels
	
	cInteractionDataResolved() {}
	cInteractionDataResolved(const json::value& v) :
	Users([](const json::value& v) {
		try {
			return tUserMap(v.at("users"));
		}
		catch (const std::exception&) {
			return tUserMap();
		}
	} (v)),
	Members([](const json::value& v) {
		try {
			return tMemberMap(v.at("members"));
		}
		catch (const std::exception&) {
			return tMemberMap();
		}
	} (v)) {}
};
typedef   hHandle<cInteractionDataResolved>   hInteractionDataResolved;
typedef  chHandle<cInteractionDataResolved>  chInteractionDataResolved;
typedef  uhHandle<cInteractionDataResolved>  uhInteractionDataResolved;
typedef uchHandle<cInteractionDataResolved> uchInteractionDataResolved;
typedef  shHandle<cInteractionDataResolved>  shInteractionDataResolved;
typedef schHandle<cInteractionDataResolved> schInteractionDataResolved;

/* ================================================================================================= */
class cInteractionDataOption final {
private:
	std::string             name;  // The name of the option
	eApplicationCommandType type;  // The type of the value
	json::value             value; // The value of the option
	// options
	
	/* Match eApplicationCommandType to a return type */
	template<eApplicationCommandType> struct _t {};
	template<> struct _t<APP_COMMAND_STRING>  { typedef const char* Type; };
	template<> struct _t<APP_COMMAND_INTEGER> { typedef int         Type; };
	template<> struct _t<APP_COMMAND_BOOLEAN> { typedef bool        Type; };
	template<> struct _t<APP_COMMAND_USER>    { typedef chSnowflake Type; };
	template<> struct _t<APP_COMMAND_CHANNEL> { typedef chSnowflake Type; };
	template<> struct _t<APP_COMMAND_ROLE>    { typedef chSnowflake Type; };
	template<> struct _t<APP_COMMAND_NUMBER>  { typedef double      Type; };
	
	template<eApplicationCommandType t>
	using tCommandType = typename _t<t>::Type;
	
	uchSnowflake m_snowflake;
	
public:
	cInteractionDataOption(const json::value&);
	
	const char*             GetName() const { return name.c_str(); }
	eApplicationCommandType GetType() const { return type;         }
	
	template<eApplicationCommandType t>
	tCommandType<t> GetValue() const {
		if constexpr (t == APP_COMMAND_STRING) {
			auto p = value.if_string();
			return p && t == type ? p->c_str() : nullptr;
		}
		else if constexpr (t == APP_COMMAND_INTEGER) {
			auto p = value.if_int64();
			return p && t == type ? *p : 0;
		}
		else if constexpr (t == APP_COMMAND_BOOLEAN) {
			auto p = value.if_bool();
			return p && t == type ? *p : false;
		}
		else if constexpr (t == APP_COMMAND_NUMBER) {
			auto p = value.if_double();
			return p && t == type ? *p : 0.0;
		}
		else {
			return m_snowflake.get();
		}
	}
};
typedef   hHandle<cInteractionDataOption>   hInteractionDataOption;
typedef  chHandle<cInteractionDataOption>  chInteractionDataOption;
typedef  uhHandle<cInteractionDataOption>  uhInteractionDataOption;
typedef uchHandle<cInteractionDataOption> uchInteractionDataOption;
typedef  shHandle<cInteractionDataOption>  shInteractionDataOption;
typedef schHandle<cInteractionDataOption> schInteractionDataOption;

/* ================================================================================================= */
class cInteractionData final {
private:
	cSnowflake                          id;
	std::string                         name;
	cInteractionDataResolved            resolved;
	std::vector<cInteractionDataOption> options;
	// custom_id
	// component_type
	
public:
	const std::vector<chInteractionDataOption> Options;
	
	cInteractionData(const json::value&);
	
	chSnowflake               GetCommandId()    const { return &id;          }
	const char*               GetCommandName()  const { return name.c_str(); }
	chInteractionDataResolved GetResolvedData() const { return &resolved;    }
	
};
typedef   hHandle<cInteractionData>   hInteractionData; // handle
typedef  chHandle<cInteractionData>  chInteractionData; // const handle
typedef  uhHandle<cInteractionData>  uhInteractionData; // unique handle
typedef uchHandle<cInteractionData> uchInteractionData; // unique const handle
typedef  shHandle<cInteractionData>  shInteractionData; // shared handle
typedef schHandle<cInteractionData> schInteractionData; // shared const handle

/* ================================================================================================= */
class cInteraction final {
private:
	cSnowflake       id;
	cSnowflake       application_id;
	eInteractionType type;
	cInteractionData data;
	cSnowflake       guild_id;
	cSnowflake       channel_id;
	uchMember        member;
	uchUser          user;
	std::string      token;
	int              version;
	// message
	
public:
	cInteraction(const json::value& v) :
	id(v.at("id").as_string().c_str()),
	application_id(v.at("application_id").as_string().c_str()),
	type(static_cast<eInteractionType>(v.at("type").as_int64())),
	data(v.at("data")),
	guild_id(v.at("guild_id").as_string().c_str()),
	channel_id(v.at("channel_id").as_string().c_str()),
	member([](const json::value& v) {
		try {
			return std::make_unique<const cMember>(v.at("member"));
		}
		catch (const std::exception&) {
			return uchMember();
		}
	} (v)),
	user([](const json::value& v) {
		try {
			return std::make_unique<const cUser>(v.at("user"));
		}
		catch (const std::exception&) {
			return uchUser();
		}
	} (v)),
	token(v.at("token").as_string().c_str()),
	version(static_cast<int>(v.at("version").as_int64())) {}
	
	chSnowflake       GetId()            const { return &id;             }
	chSnowflake       GetApplicationId() const { return &application_id; }
	eInteractionType  GetType()          const { return type;            }
	chInteractionData GetData()          const { return &data;           }
	chSnowflake       GetGuildId()       const { return &guild_id;       }
	chSnowflake       GetChannelId()     const { return &channel_id;     }
	chMember          GetMember()        const { return member.get();    }
	const char*       GetToken()         const { return token.c_str();   }
	int               GetVersion()       const { return version;         }
};
typedef   hHandle<cInteraction>   hInteraction; // handle
typedef  chHandle<cInteraction>  chInteraction; // const handle
typedef  uhHandle<cInteraction>  uhInteraction; // unique handle
typedef uchHandle<cInteraction> uchInteraction; // unique const handle
typedef  shHandle<cInteraction>  shInteraction; // shared handle
typedef schHandle<cInteraction> schInteraction; // shared const handle

#endif /* _GREEKBOT_INTERACTION_H_ */
