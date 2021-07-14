#pragma once
#ifndef _GREEKBOT_JSONERROR_H_
#define _GREEKBOT_JSONERROR_H_
#include "json.h"
#include <string>

enum eJsonErrorCode {
	DISCORD_JSON_UNKNOWN_ERROR = -1,
	DISCORD_JSON_GENERAL_ERROR = 0
};

class cJsonError final {
private:
	eJsonErrorCode m_code;
	const char* m_message;
	
	void Init(const json::value&);
	
public:
	cJsonError(const json::value& v);
	cJsonError(const cJsonError& ) = delete;
	cJsonError(      cJsonError&&) = delete;
	
	const char* GetMessage() const { return m_message; }
	eJsonErrorCode GetCode() const { return m_code; }
};

#endif /* _GREEKBOT_JSONERROR_H_*/
