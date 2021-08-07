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
	template<> struct _t<APP_COMMAND_USER>    { typedef chUser      Type; };
	template<> struct _t<APP_COMMAND_CHANNEL> { typedef chSnowflake Type; };
	template<> struct _t<APP_COMMAND_ROLE>    { typedef chSnowflake Type; };
	template<> struct _t<APP_COMMAND_NUMBER>  { typedef double      Type; };
	
	template<eApplicationCommandType t>
	using tCommandType = typename _t<t>::Type;

	uchUser      m_user;      // The resolved user object of the value
	uchMember    m_member;    // The resolved member object of the value
	uchSnowflake m_snowflake; // The resolved snowflake object of the value
	
public:
	cInteractionDataOption(const json::value& v, const json::value* r);
	
	[[nodiscard]] const char*             GetName() const { return name.c_str(); }
	[[nodiscard]] eApplicationCommandType GetType() const { return type;         }
	
	template<eApplicationCommandType t>
	[[nodiscard]] tCommandType<t> GetValue() const {
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
			return p && t == type && *p;
		}
		else if constexpr (t == APP_COMMAND_NUMBER) {
			auto p = value.if_double();
			return p && t == type ? *p : 0.0;
		}
		else if constexpr (t == APP_COMMAND_USER)
			return m_user.get();
		else
			return m_snowflake.get();
	}

	template<eApplicationCommandType t>
	[[nodiscard]] tCommandType<t> GetValue(chMember&) const { /* Expect compilation error for missing return value */ }

	template<>
	[[nodiscard]] tCommandType<APP_COMMAND_USER> GetValue<APP_COMMAND_USER>(chMember& m) const {
		m = m_member.get();
		return m_user.get();
	}
};
typedef   hHandle<cInteractionDataOption>   hInteractionDataOption;
typedef  chHandle<cInteractionDataOption>  chInteractionDataOption;
typedef  uhHandle<cInteractionDataOption>  uhInteractionDataOption;
typedef uchHandle<cInteractionDataOption> uchInteractionDataOption;
typedef  shHandle<cInteractionDataOption>  shInteractionDataOption;
typedef schHandle<cInteractionDataOption> schInteractionDataOption;

/* ================================================================================================= */
class cInteractionData {
private:
	cSnowflake                          id;
	std::string                         name;
	std::vector<cInteractionDataOption> options;
	// custom_id
	// component_type
public:
	const std::vector<chInteractionDataOption> Options;

	explicit cInteractionData(const json::value& v);

	[[nodiscard]] chSnowflake GetCommandId()   const { return &id;          }
	[[nodiscard]] const char* GetCommandName() const { return name.c_str(); }
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
	uchSnowflake     guild_id;
	uchSnowflake     channel_id;
	uchMember        member;
	uchUser          user;
	std::string      token;
	int              version;
	// message

public:
	explicit cInteraction(const json::value& v);
	
	[[nodiscard]] chSnowflake       GetId()            const { return &id;             }
	[[nodiscard]] chSnowflake       GetApplicationId() const { return &application_id; }
	[[nodiscard]] eInteractionType  GetType()          const { return type;            }
	[[nodiscard]] chInteractionData GetData()          const { return &data;           }
	[[nodiscard]] chSnowflake       GetGuildId()       const { return guild_id   ?   guild_id.get() : nullptr; }
	[[nodiscard]] chSnowflake       GetChannelId()     const { return channel_id ? channel_id.get() : nullptr; }
	[[nodiscard]] chMember          GetMember()        const { return member     ?     member.get() : nullptr; }
	[[nodiscard]] chUser            GetUser()          const { return user       ?       user.get() : nullptr; }
	[[nodiscard]] const char*       GetToken()         const { return token.c_str(); }
	[[nodiscard]] int               GetVersion()       const { return version;       }
};
typedef   hHandle<cInteraction>   hInteraction; // handle
typedef  chHandle<cInteraction>  chInteraction; // const handle
typedef  uhHandle<cInteraction>  uhInteraction; // unique handle
typedef uchHandle<cInteraction> uchInteraction; // unique const handle
typedef  shHandle<cInteraction>  shInteraction; // shared handle
typedef schHandle<cInteraction> schInteraction; // shared const handle
#endif /* _GREEKBOT_INTERACTION_H_ */