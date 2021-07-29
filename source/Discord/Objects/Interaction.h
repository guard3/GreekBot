#pragma once
#ifndef _GREEKBOT_INTERACTION_H_
#define _GREEKBOT_INTERACTION_H_
#include "Types.h"
#include "json.h"

enum eInteractionType {
	INTERACTION_PING                = 1,
	INTERACTION_APPLICATION_COMMAND = 2,
	INTERACTION_MESSAGE_COMPONENT   = 3
};

class cInteractionData final {
private:
	cSnowflake id;
	char name[32];
	// resolved
	// options
	// custom_id
	// component_type
	
public:
	cInteractionData(const json::value& v) : id(v.at("id").as_string().c_str()) {
		strcpy(name, v.at("name").as_string().c_str());
	}
	
	chSnowflake GetCommandId()   const { return &id;  }
	const char* GetCommandName() const { return name; }
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
	json::string token;
	int          version;
	// message
	
public:
	cInteraction(const json::value& v) :
		id(v.at("id").as_string().c_str()),
		application_id(v.at("application_id").as_string().c_str()),
		type(static_cast<eInteractionType>(v.at("type").as_int64())),
		data(v.at("data")),
		guild_id(v.at("guild_id").as_string().c_str()),
		channel_id(v.at("channel_id").as_string().c_str()),
		token(v.at("token").as_string()),
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
