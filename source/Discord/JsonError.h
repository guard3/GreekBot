#pragma once
#ifndef _GREEKBOT_JSONERROR_H_
#define _GREEKBOT_JSONERROR_H_
#include "json.h"

enum eJsonErrorCode {
	DISCORD_JSON_UNKNOWN_ERROR = -1,
	DISCORD_JSON_GENERAL_ERROR = 0
};

class cJsonError final {
private:
	eJsonErrorCode code;
	json::string   message;
	
public:
	/* Initialize from JSON value - throws exceptions */
	cJsonError(const json::value& v) : code(static_cast<eJsonErrorCode>(v.at("code").as_int64())), message(v.at("message").as_string()) {}
	
	/* Attributes */
	eJsonErrorCode GetCode() const { return code;            }
	const char* GetMessage() const { return message.c_str(); }
};

typedef const cJsonError* hJsonError;

#endif /* _GREEKBOT_JSONERROR_H_*/
