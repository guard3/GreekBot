#pragma once
#ifndef _GREEKBOT_INTERACTION_H_
#define _GREEKBOT_INTERACTION_H_
#include "Types.h"
#include "json.h"
#include "User.h"
#include "Member.h"
#include <vector>
#include "Component.h"

/* ================================================================================================= */
enum eInteractionType {
	INTERACTION_PING = 1,
	INTERACTION_APPLICATION_COMMAND,
	INTERACTION_MESSAGE_COMPONENT
};

/* ================================================================================================= */
enum eApplicationCommandType {
	APP_COMMAND_CHAT_INPUT = 1,
	APP_COMMAND_USER,
	APP_COMMAND_MESSAGE
};

/* ================================================================================================= */
enum eApplicationCommandOptionType {
	APP_COMMAND_OPT_SUB_COMMAND = 1,
	APP_COMMAND_OPT_SUB_COMMAND_GROUP,
	APP_COMMAND_OPT_STRING,
	APP_COMMAND_OPT_INTEGER,
	APP_COMMAND_OPT_BOOLEAN,
	APP_COMMAND_OPT_USER,
	APP_COMMAND_OPT_CHANNEL,
	APP_COMMAND_OPT_ROLE,
	APP_COMMAND_OPT_MENTIONABLE,
	APP_COMMAND_OPT_NUMBER
};

/* ================================================================================================= */
class cApplicationCommandInteractionDataOption final {
private:
	char*                         name;
	eApplicationCommandOptionType type;

	union {
		char*       v_str;
		chSnowflake v_sfl;
		int         v_int;
		bool        v_bll;
		double      v_dbl;
		struct {
			chUser   v_usr;
			chMember v_mbr;
		} v_usr;
	} value;

	template<eApplicationCommandOptionType> struct _t {};
	template<> struct _t<APP_COMMAND_OPT_STRING>  { typedef const char* Type; };
	template<> struct _t<APP_COMMAND_OPT_INTEGER> { typedef int         Type; };
	template<> struct _t<APP_COMMAND_OPT_BOOLEAN> { typedef bool        Type; };
	template<> struct _t<APP_COMMAND_OPT_USER>    { typedef chUser      Type; };
	template<> struct _t<APP_COMMAND_OPT_CHANNEL> { typedef chSnowflake Type; };
	template<> struct _t<APP_COMMAND_OPT_ROLE>    { typedef chSnowflake Type; };
	template<> struct _t<APP_COMMAND_OPT_NUMBER>  { typedef double      Type; };
	template<eApplicationCommandOptionType t>
	using tValueType = typename _t<t>::Type;
	
public:
	cApplicationCommandInteractionDataOption(const json::value& v, const json::value* r);
	cApplicationCommandInteractionDataOption(const cApplicationCommandInteractionDataOption&);
	cApplicationCommandInteractionDataOption(cApplicationCommandInteractionDataOption&&) noexcept;
	~cApplicationCommandInteractionDataOption();

	cApplicationCommandInteractionDataOption& operator=(cApplicationCommandInteractionDataOption);
	
	[[nodiscard]] const char*                   GetName() const { return name; }
	[[nodiscard]] eApplicationCommandOptionType GetType() const { return type; }

	template<eApplicationCommandOptionType t>
	[[nodiscard]] tValueType<t> GetValue() const {
		     if constexpr (t == APP_COMMAND_OPT_STRING)  return t == type ?  value.v_str : nullptr;
		else if constexpr (t == APP_COMMAND_OPT_INTEGER) return t == type ?  value.v_int : 0;
		else if constexpr (t == APP_COMMAND_OPT_NUMBER)  return t == type ?  value.v_dbl : 0.0;
		else if constexpr (t == APP_COMMAND_OPT_BOOLEAN) return t == type && value.v_bll;
		else if constexpr (t == APP_COMMAND_OPT_USER)    return t == type ?  value.v_usr.v_usr : nullptr;
		else                                             return t == type ?  value.v_sfl       : nullptr;
	}
	template<eApplicationCommandOptionType t>
	[[nodiscard]] tValueType<t> GetValue(chMember&) const { /* Expect compilation error for missing return value */ }
	template<>
	[[nodiscard]] tValueType<APP_COMMAND_OPT_USER> GetValue<APP_COMMAND_OPT_USER>(chMember& m) const {
		if (type == APP_COMMAND_OPT_USER) {
			m = value.v_usr.v_mbr;
			return value.v_usr.v_usr;
		}
		m = nullptr;
		return nullptr;
	}
};
typedef   hHandle<cApplicationCommandInteractionDataOption>   hApplicationCommandInteractionDataOption;
typedef  chHandle<cApplicationCommandInteractionDataOption>  chApplicationCommandInteractionDataOption;
typedef  uhHandle<cApplicationCommandInteractionDataOption>  uhApplicationCommandInteractionDataOption;
typedef uchHandle<cApplicationCommandInteractionDataOption> uchApplicationCommandInteractionDataOption;

/* ================================================================================================= */
class cApplicationCommandInteractionData {
private:
	cSnowflake              id;                  // The id of the invoked command
	char*                   name = nullptr;      // The name of the invoked command
	eApplicationCommandType type;                // The type of the invoked command
	chSnowflake             target_id = nullptr; // The id the of user or message targeted by a user or message command

public:
	const std::vector<chApplicationCommandInteractionDataOption> Options;

	explicit cApplicationCommandInteractionData(const json::value&);
	cApplicationCommandInteractionData(const cApplicationCommandInteractionData&);
	cApplicationCommandInteractionData(cApplicationCommandInteractionData&&) noexcept;
	~cApplicationCommandInteractionData();

	[[nodiscard]] chSnowflake             GetCommandId()   const { return &id;       }
	[[nodiscard]] const char*             GetCommandName() const { return name;      }
	[[nodiscard]] eApplicationCommandType GetCommandType() const { return type;      }
	[[nodiscard]] chSnowflake             GetTargetId()    const { return target_id; }
};
typedef   hHandle<cApplicationCommandInteractionData>   hApplicationCommandInteractionData; // handle
typedef  chHandle<cApplicationCommandInteractionData>  chApplicationCommandInteractionData; // const handle
typedef  uhHandle<cApplicationCommandInteractionData>  uhApplicationCommandInteractionData; // unique handle
typedef uchHandle<cApplicationCommandInteractionData> uchApplicationCommandInteractionData; // unique const handle

/* ================================================================================================= */
class cMessageComponentInteractionData final {
private:
	char*          custom_id;      // The custom_id of the component
	eComponentType component_type; // The type of the component

public:
	const std::vector<const char*> Values; // The values the user selected (select menu)

	explicit cMessageComponentInteractionData(const json::value&);
	cMessageComponentInteractionData(const cMessageComponentInteractionData&);
	cMessageComponentInteractionData(cMessageComponentInteractionData&& o) noexcept;
	~cMessageComponentInteractionData();

	cMessageComponentInteractionData& operator=(cMessageComponentInteractionData o);

	[[nodiscard]] const char*    GetCustomId()      const { return custom_id;      }
	[[nodiscard]] eComponentType GetComponentType() const { return component_type; }
};
typedef   hHandle<cMessageComponentInteractionData>   hMessageComponentInteractionData; // handle
typedef  chHandle<cMessageComponentInteractionData>  chMessageComponentInteractionData; // const handle
typedef  uhHandle<cMessageComponentInteractionData>  uhMessageComponentInteractionData; // unique handle
typedef uchHandle<cMessageComponentInteractionData> uchMessageComponentInteractionData; // unique const handle

/* ================================================================================================= */
class cInteraction final {
private:
	cSnowflake       id;
	cSnowflake       application_id;
	eInteractionType type;
	void*            data = nullptr;
	uchSnowflake     guild_id;
	uchSnowflake     channel_id;
	uchMember        member;
	uchUser          user;
	std::string      token;
	int              version;
	// message

	template<eInteractionType> struct _t {};
	template<> struct _t<INTERACTION_APPLICATION_COMMAND> { typedef chApplicationCommandInteractionData Type; };
	template<> struct _t<INTERACTION_MESSAGE_COMPONENT>   { typedef chMessageComponentInteractionData   Type; };
	template<eInteractionType t>
	using tInteractionDataType = typename _t<t>::Type;

public:
	explicit cInteraction(const json::value& v);
	
	[[nodiscard]] chSnowflake       GetId()            const { return &id;             }
	[[nodiscard]] chSnowflake       GetApplicationId() const { return &application_id; }
	[[nodiscard]] eInteractionType  GetType()          const { return type;            }
	//[[nodiscard]] chInteractionData GetData()          const { return &data;           }
	[[nodiscard]] chSnowflake       GetGuildId()       const { return guild_id   ?   guild_id.get() : nullptr; }
	[[nodiscard]] chSnowflake       GetChannelId()     const { return channel_id ? channel_id.get() : nullptr; }
	[[nodiscard]] chMember          GetMember()        const { return member     ?     member.get() : nullptr; }
	[[nodiscard]] chUser            GetUser()          const { return user       ?       user.get() : nullptr; }
	[[nodiscard]] const char*       GetToken()         const { return token.c_str(); }
	[[nodiscard]] int               GetVersion()       const { return version;       }

	template<eInteractionType t>
	[[nodiscard]] tInteractionDataType<t> GetData() const { return reinterpret_cast<tInteractionDataType<t>>(data); }
};
typedef   hHandle<cInteraction>   hInteraction; // handle
typedef  chHandle<cInteraction>  chInteraction; // const handle
typedef  uhHandle<cInteraction>  uhInteraction; // unique handle
typedef uchHandle<cInteraction> uchInteraction; // unique const handle
typedef  shHandle<cInteraction>  shInteraction; // shared handle
typedef schHandle<cInteraction> schInteraction; // shared const handle
#endif /* _GREEKBOT_INTERACTION_H_ */