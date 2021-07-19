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
	eJsonErrorCode m_code;
	char* m_message;
	
public:
	/* Initialize from JSON value - throws exceptions */
	cJsonError(const json::object&);
	cJsonError(const cJsonError&);
	cJsonError(cJsonError&&);
	~cJsonError();
	
	/* Assignment operator */
	cJsonError& operator=(cJsonError);
	
	/* Attributes */
	const char* GetMessage() const { return m_message; }
	eJsonErrorCode GetCode() const { return m_code;    }
};

typedef const cJsonError* hJsonError;

#endif /* _GREEKBOT_JSONERROR_H_*/
