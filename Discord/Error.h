#pragma once
#ifndef _GREEKBOT_JSONERROR_H_
#define _GREEKBOT_JSONERROR_H_
#include "json.h"
#include "Types.h"

enum eErrorCode {
	ERROR_GENERIC = 0
};

class cError final {
private:
	eErrorCode  code;
	std::string message;
	
public:
	/* Initialize from JSON value - throws exceptions */
	explicit cError(const json::value& v) : code(static_cast<eErrorCode>(v.at("code").as_int64())), message(v.at("message").as_string().c_str()) {}
	
	/* Attributes */
	[[nodiscard]] eErrorCode  GetCode()    const { return code;            }
	[[nodiscard]] const char *GetMessage() const { return message.c_str(); }
};
typedef   hHandle<cError>   hError;
typedef  chHandle<cError>  chError;
typedef  uhHandle<cError>  uhError;
typedef uchHandle<cError> uchError;
typedef  shHandle<cError>  shError;
typedef schHandle<cError> schError;
#endif /* _GREEKBOT_JSONERROR_H_*/
