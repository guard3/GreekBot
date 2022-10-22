#pragma once
#ifndef _GREEKBOT_INTERACTION_H_
#define _GREEKBOT_INTERACTION_H_
#include "Common.h"
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
	APP_CMD_CHAT_INPUT = 1,
	APP_CMD_USER,
	APP_CMD_MESSAGE
};

/* ================================================================================================= */
enum eApplicationCommandOptionType {
	APP_CMD_OPT_SUB_COMMAND = 1,
	APP_CMD_OPT_SUB_COMMAND_GROUP,
	APP_CMD_OPT_STRING,
	APP_CMD_OPT_INTEGER,
	APP_CMD_OPT_BOOLEAN,
	APP_CMD_OPT_USER,
	APP_CMD_OPT_CHANNEL,
	APP_CMD_OPT_ROLE,
	APP_CMD_OPT_MENTIONABLE,
	APP_CMD_OPT_NUMBER
};

/* ================================================================================================= */
class xInvalidAttributeError : public std::runtime_error {
public:
	xInvalidAttributeError(const char* m)        : std::runtime_error(m) {}
	xInvalidAttributeError(const std::string& m) : std::runtime_error(m) {}
};

class cApplicationCommandOption final {
private:
	template<eApplicationCommandOptionType> struct a;

	std::string                   m_name;
	eApplicationCommandOptionType m_type;

	union {
		std::vector<cApplicationCommandOption> m_options;
		std::string m_value_string;
		cSnowflake  m_value_snowflake;
		int         m_value_integer;
		bool        m_value_boolean;
		double      m_value_number;
		std::tuple<cUser, cWrapper<cMember>> m_value_user;
	};
	
public:
	/* Return types */
	template<eApplicationCommandOptionType t>
	using tMoveType = typename a<t>::value_type;
	template<eApplicationCommandOptionType t>
	using tValueType = std::conditional_t<t == APP_CMD_OPT_INTEGER || t == APP_CMD_OPT_BOOLEAN || t == APP_CMD_OPT_NUMBER, tMoveType<t>, tMoveType<t>&>;
	template<eApplicationCommandOptionType t>
	using tConstValueType = std::conditional_t<std::is_reference_v<tValueType<t>>, const tMoveType<t>&, tMoveType<t>>;
	/* Constructors */
	cApplicationCommandOption(const json::value& v, const json::value* r);
	cApplicationCommandOption(const cApplicationCommandOption&);
	cApplicationCommandOption(cApplicationCommandOption&&) noexcept;
	~cApplicationCommandOption();
	cApplicationCommandOption& operator=(cApplicationCommandOption);
	/* Non const getters */
	std::string& GetName() noexcept { return m_name; }
	template<eApplicationCommandOptionType t>
	tValueType<t> GetValue() {
		if (t != m_type) throw xInvalidAttributeError(cUtils::Format("Application command option is not of type %s", a<t>::name));
		     if constexpr (t == APP_CMD_OPT_STRING ) return m_value_string;
		else if constexpr (t == APP_CMD_OPT_INTEGER) return m_value_integer;
		else if constexpr (t == APP_CMD_OPT_BOOLEAN) return m_value_boolean;
		else if constexpr (t == APP_CMD_OPT_USER   ) return std::get<0>(m_value_user);
		else if constexpr (t == APP_CMD_OPT_NUMBER ) return m_value_number;
		else                                         return m_value_snowflake;
	}
	hMember GetMember();
	std::vector<cApplicationCommandOption>& GetOptions();
	/* Const getters */
	const std::string&            GetName() const noexcept { return m_name; }
	eApplicationCommandOptionType GetType() const noexcept { return m_type; }
	template<eApplicationCommandOptionType t>
	tConstValueType<t> GetValue() const { return const_cast<cApplicationCommandOption*>(this)->template GetValue<t>(); }
	chMember GetMember() const { return const_cast<cApplicationCommandOption*>(this)->GetMember(); }
	const std::vector<cApplicationCommandOption>& GetOptions() const { return const_cast<cApplicationCommandOption*>(this)->GetOptions(); }
	/* Movers */
	std::string MoveName() noexcept { return m_name; }
	template<eApplicationCommandOptionType t>
	tMoveType<t> MoveValue() { return std::move(GetValue<t>()); }
	uhMember MoveMember();
	std::vector<cApplicationCommandOption> MoveOptions() { return std::move(GetOptions()); }
};
template<> struct cApplicationCommandOption::a<APP_CMD_OPT_STRING>      { using value_type = std::string; static inline auto name = "APP_CMD_OPT_STRING";      };
template<> struct cApplicationCommandOption::a<APP_CMD_OPT_INTEGER>     { using value_type = int;         static inline auto name = "APP_CMD_OPT_INTEGER";     };
template<> struct cApplicationCommandOption::a<APP_CMD_OPT_BOOLEAN>     { using value_type = bool;        static inline auto name = "APP_CMD_OPT_BOOLEAN";     };
template<> struct cApplicationCommandOption::a<APP_CMD_OPT_USER>        { using value_type = cUser;       static inline auto name = "APP_CMD_OPT_USER";        };
template<> struct cApplicationCommandOption::a<APP_CMD_OPT_CHANNEL>     { using value_type = cSnowflake;  static inline auto name = "APP_CMD_OPT_CHANNEL";     };
template<> struct cApplicationCommandOption::a<APP_CMD_OPT_ROLE>        { using value_type = cSnowflake;  static inline auto name = "APP_CMD_OPT_ROLE";        };
template<> struct cApplicationCommandOption::a<APP_CMD_OPT_MENTIONABLE> { using value_type = cSnowflake;  static inline auto name = "APP_CMD_OPT_MENTIONABLE"; };
template<> struct cApplicationCommandOption::a<APP_CMD_OPT_NUMBER>      { using value_type = double;      static inline auto name = "APP_CMD_OPT_NUMBER";      };

typedef   hHandle<cApplicationCommandOption>   hApplicationCommandInteractionDataOption;
typedef  chHandle<cApplicationCommandOption>  chApplicationCommandInteractionDataOption;
typedef  uhHandle<cApplicationCommandOption>  uhApplicationCommandInteractionDataOption;
typedef uchHandle<cApplicationCommandOption> uchApplicationCommandInteractionDataOption;
typedef  uhHandle<cApplicationCommandOption>  shApplicationCommandInteractionDataOption;
typedef uchHandle<cApplicationCommandOption> schApplicationCommandInteractionDataOption;

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
	std::vector<cApplicationCommandOption> Options;

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