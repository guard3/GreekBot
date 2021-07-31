#pragma once
#ifndef _GREEKBOT_INTERACTION_H_
#define _GREEKBOT_INTERACTION_H_
#include "Types.h"
#include "json.h"
#include "Utils.h"
#include <sstream>
#include <vector>

enum eInteractionType {
	INTERACTION_PING = 1,
	INTERACTION_APPLICATION_COMMAND,
	INTERACTION_MESSAGE_COMPONENT
};

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

class cInteractionResolvedData final {
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
	typedef cMap<cUser> tUserMap;
	
private:
	tUserMap InitUserMap(const json::value& v) {
		try {
			return tUserMap(v.at("users"));
		}
		catch (const std::exception&) {
			return tUserMap();
		}
	}
	
public:
	const tUserMap Users;
	// members
	// roles
	// channels
	
	cInteractionResolvedData() {}
	cInteractionResolvedData(const json::value& v) : Users(InitUserMap(v)) {}
};
typedef   hHandle<cInteractionResolvedData>   hInteractionResolvedData;
typedef  chHandle<cInteractionResolvedData>  chInteractionResolvedData;
typedef  uhHandle<cInteractionResolvedData>  uhInteractionResolvedData;
typedef uchHandle<cInteractionResolvedData> uchInteractionResolvedData;
typedef  shHandle<cInteractionResolvedData>  shInteractionResolvedData;
typedef schHandle<cInteractionResolvedData> schInteractionResolvedData;


class cInteractionDataOption final {
private:
	std::string             name;  // The name of the option
	eApplicationCommandType type;  // The type of the value
	const json::value       value; // The value of the option
	// options
	
	/* A struct that matches eApplicationCommandType to a return type */
	template<eApplicationCommandType> struct _t {};
	template<> struct _t<APP_COMMAND_STRING>  { typedef const char* Type; };
	template<> struct _t<APP_COMMAND_INTEGER> { typedef int         Type; };
	template<> struct _t<APP_COMMAND_BOOLEAN> { typedef bool        Type; };
	template<> struct _t<APP_COMMAND_NUMBER>  { typedef double      Type; };
	
	template<eApplicationCommandType t>
	using tCommandType = typename _t<t>::Type;
	
public:
	cInteractionDataOption(const json::value& v) : name(v.at("name").as_string().c_str()), type(static_cast<eApplicationCommandType>(v.at("type").as_int64())), value(v.at("value")) {}
	
	const char* GetName() const { return name.c_str(); }
	eApplicationCommandType GetType() const { return type; }
	
	template<eApplicationCommandType t>
	auto GetValue() const {
		try {
			if constexpr (t == APP_COMMAND_STRING)
				return value.as_string().c_str();
			else if constexpr (t == APP_COMMAND_INTEGER)
				return static_cast<int>(value.as_int64());
			else if constexpr (t == APP_COMMAND_BOOLEAN)
				return value.as_bool();
			else if constexpr (t == APP_COMMAND_NUMBER)
				return value.as_double();
			else
				return std::make_unique<const cSnowflake>(value.as_string().c_str());
		}
		catch (const std::exception&) {
			if constexpr (t == APP_COMMAND_USER || t == APP_COMMAND_CHANNEL || t == APP_COMMAND_ROLE)
				return uchSnowflake();
			else
				return static_cast<tCommandType<t>>(0);
		}
	}
	
};
typedef   hHandle<cInteractionDataOption>   hInteractionDataOption;
typedef  chHandle<cInteractionDataOption>  chInteractionDataOption;
typedef  uhHandle<cInteractionDataOption>  uhInteractionDataOption;
typedef uchHandle<cInteractionDataOption> uchInteractionDataOption;
typedef  shHandle<cInteractionDataOption>  shInteractionDataOption;
typedef schHandle<cInteractionDataOption> schInteractionDataOption;

class cInteractionData final {
private:
	cSnowflake id;
	char name[32];
	cInteractionResolvedData resolved;
	std::vector<cInteractionDataOption> options;
	// custom_id
	// component_type
	
	static auto init_resolved(const json::value& v) {
		try {
			return cInteractionResolvedData(v.at("resolved"));
		}
		catch (const std::exception&) {
			return cInteractionResolvedData();
		}
	}
	
	static auto init_options(const json::value& v) {
		try {
			const json::array& a = v.at("options").as_array();
			std::vector<cInteractionDataOption> result;
			result.reserve(a.size());
			for (const json::value& val : a)
				result.push_back(cInteractionDataOption(val));
			return result;
		}
		catch (const std::exception&) {
			return std::vector<cInteractionDataOption>();
		}
	}
	
public:
	std::vector<chInteractionDataOption> Options;
	
	cInteractionData(const json::value& v) : id(v.at("id").as_string().c_str()), resolved(init_resolved(v)) {
		try {
			const json::array& a = v.at("options").as_array();
			//options.reserve(a.size());
			//Options.reserve(a.size());
			for (const json::value& val : a) {
				std::stringstream s;
				s << val;
				cUtils::PrintLog(s.str().c_str());
				options.push_back(cInteractionDataOption(val));
				Options.push_back(&options.back());
			}
		}
		catch (const std::exception&) {
			options.clear();
			Options.clear();
		}
		
		strcpy(name, v.at("name").as_string().c_str());
	}
	
	chSnowflake GetCommandId()   const { return &id;  }
	const char* GetCommandName() const { return name; }
	chInteractionResolvedData GetResolvedData() const { return &resolved; }
	
};
typedef   hHandle<cInteractionData>   hInteractionData; // handle
typedef  chHandle<cInteractionData>  chInteractionData; // const handle
typedef  uhHandle<cInteractionData>  uhInteractionData; // unique handle
typedef uchHandle<cInteractionData> uchInteractionData; // unique const handle
typedef  shHandle<cInteractionData>  shInteractionData; // shared handle
typedef schHandle<cInteractionData> schInteractionData; // shared const handle

class cInteraction final {
private:
	cSnowflake       id;
	cSnowflake       application_id;
	eInteractionType type;
	cInteractionData data;
	cSnowflake       guild_id;
	cSnowflake       channel_id;
	// member
	// user
	std::string token;
	int         version;
	// message
	
public:
	cInteraction(const json::value& v) :
		id(v.at("id").as_string().c_str()),
		application_id(v.at("application_id").as_string().c_str()),
		type(static_cast<eInteractionType>(v.at("type").as_int64())),
		data(v.at("data")),
		guild_id(v.at("guild_id").as_string().c_str()),
		channel_id(v.at("channel_id").as_string().c_str()),
		token(v.at("token").as_string().c_str()),
		version(static_cast<int>(v.at("version").as_int64())) {}
	
	chSnowflake       GetId()            const { return &id;             }
	chSnowflake       GetApplicationId() const { return &application_id; }
	eInteractionType  GetType()          const { return type;            }
	chInteractionData GetData()          const { return &data;           }
	chSnowflake       GetGuildId()       const { return &guild_id;       }
	chSnowflake       GetChannelId()     const { return &channel_id;     }
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
