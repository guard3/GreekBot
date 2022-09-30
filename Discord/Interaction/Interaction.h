#pragma once
#ifndef _GREEKBOT_INTERACTION_H_
#define _GREEKBOT_INTERACTION_H_
#include "Common.h"
#include "json.h"
#include "User.h"
#include "Member.h"
#include <vector>
#include "Component.h"
#include "Message.h"

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

class cApplicationCommandInteractionDataOption final {
private:
	std::string                   name;
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
	
public:
	cApplicationCommandInteractionDataOption(const json::value& v, const json::value* r);
	cApplicationCommandInteractionDataOption(const cApplicationCommandInteractionDataOption&);
	cApplicationCommandInteractionDataOption(cApplicationCommandInteractionDataOption&&) noexcept;
	~cApplicationCommandInteractionDataOption();

	cApplicationCommandInteractionDataOption& operator=(cApplicationCommandInteractionDataOption);
	
	[[nodiscard]] const char*                   GetName() const noexcept { return name.c_str(); }
	[[nodiscard]] eApplicationCommandOptionType GetType() const noexcept { return type;         }

	template<eApplicationCommandOptionType t>
	[[nodiscard]] tValueType<t> GetValue() const noexcept {
		     if constexpr (t == APP_COMMAND_OPT_STRING)  return t == type ?  value.v_str : nullptr;
		else if constexpr (t == APP_COMMAND_OPT_INTEGER) return t == type ?  value.v_int : 0;
		else if constexpr (t == APP_COMMAND_OPT_NUMBER)  return t == type ?  value.v_dbl : 0.0;
		else if constexpr (t == APP_COMMAND_OPT_BOOLEAN) return t == type && value.v_bll;
		else if constexpr (t == APP_COMMAND_OPT_USER)    return t == type ?  value.v_usr.v_usr : nullptr;
		else                                             return t == type ?  value.v_sfl       : nullptr;
	}

	template<eApplicationCommandOptionType t, typename = std::enable_if_t<t == APP_COMMAND_OPT_USER>>
	tValueType<t> GetValue(chMember& m) const noexcept {
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
typedef  uhHandle<cApplicationCommandInteractionDataOption>  shApplicationCommandInteractionDataOption;
typedef uchHandle<cApplicationCommandInteractionDataOption> schApplicationCommandInteractionDataOption;

/* ================================================================================================= */
template<eInteractionType>
class cInteractionData;

template<>
class cInteractionData<INTERACTION_APPLICATION_COMMAND> final {
private:
	cSnowflake              id;        // The id of the invoked command
	std::string             name;      // The name of the invoked command
	eApplicationCommandType type;      // The type of the invoked command
	chSnowflake             target_id; // The id the of user or message targeted by a user or message command

public:
	std::vector<cApplicationCommandInteractionDataOption> Options;

	cInteractionData(const json::object&);
	cInteractionData(const json::value&);

	const cSnowflake&       GetCommandId() const { return id;        }
	const std::string&      GetName()      const { return name;      }
	eApplicationCommandType GetType()      const { return type;      }
	chSnowflake             GetTargetId()  const { return target_id; }
};

template<>
class cInteractionData<INTERACTION_MESSAGE_COMPONENT> final {
private:
	std::string    custom_id;
	eComponentType component_type;

public:
	std::vector<std::string> Values;

	cInteractionData(const json::object&);
	cInteractionData(const json::value&);

	const std::string&  GetCustomId() const noexcept { return custom_id;      }
	eComponentType GetComponentType() const noexcept { return component_type; }
};
template<eInteractionType e> using   hInteractionData =   hHandle<cInteractionData<e>>;
template<eInteractionType e> using  chInteractionData =  chHandle<cInteractionData<e>>;
template<eInteractionType e> using  uhInteractionData =  uhHandle<cInteractionData<e>>;
template<eInteractionType e> using uchInteractionData = uchHandle<cInteractionData<e>>;
template<eInteractionType e> using  shInteractionData =  shHandle<cInteractionData<e>>;
template<eInteractionType e> using schInteractionData = schHandle<cInteractionData<e>>;

/* ================================================================================================= */
class cInteraction final {
private:
	cSnowflake       id;
	cSnowflake       application_id;
	eInteractionType type;
	void*            data;
	uhUser           user;
	uhMember         member;
	uhSnowflake      guild_id;
	uhSnowflake      channel_id;
	std::string      token;
	int              version;
	uhMessage        message;

public:
	cInteraction(const json::object& o);
	cInteraction(const json::value& v);
	cInteraction(const cInteraction&);
	cInteraction(cInteraction&&) noexcept;
	~cInteraction();

	cInteraction& operator=(cInteraction);
	
	const cSnowflake&  GetId()            const { return id;               }
	const cSnowflake&  GetApplicationId() const { return application_id;   }
	eInteractionType   GetType()          const { return type;             }
	chUser             GetUser()          const { return user.get();       }
	chMember           GetMember()        const { return member.get();     }
	chSnowflake        GetGuildId()       const { return guild_id.get();   }
	chSnowflake        GetChannelId()     const { return channel_id.get(); }
	const std::string& GetToken()         const { return token;            }
	int                GetVersion()       const { return version;          }
	chMessage          GetMessage()       const { return message.get();    }

	template<eInteractionType t>
	chInteractionData<t> GetData() const { return t == type ? (chInteractionData<t>)data : nullptr;	}
};
typedef   hHandle<cInteraction>   hInteraction; // handle
typedef  chHandle<cInteraction>  chInteraction; // const handle
typedef  uhHandle<cInteraction>  uhInteraction; // unique handle
typedef uchHandle<cInteraction> uchInteraction; // unique const handle
typedef  shHandle<cInteraction>  shInteraction; // shared handle
typedef schHandle<cInteraction> schInteraction; // shared const handle
#endif /* _GREEKBOT_INTERACTION_H_ */