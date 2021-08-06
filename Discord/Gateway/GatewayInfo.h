#pragma once
#ifndef _GREEKBOT_GATEWAYINFO_H_
#define _GREEKBOT_GATEWAYINFO_H_
#include "JsonError.h"

class cSessionStartLimit final {
private:
	int m_values[4];
	
public:
	/* Construct a session_start_object from a gateway JSON - throws exceptions */
	cSessionStartLimit(const json::value& v) : m_values {
		static_cast<int>(v.at("total"          ).as_int64()),
		static_cast<int>(v.at("remaining"      ).as_int64()),
		static_cast<int>(v.at("reset_after"    ).as_int64()),
		static_cast<int>(v.at("max_concurrency").as_int64())
	} {}
	
	/* Attributes */
	int GetTotal()          const { return m_values[0]; }
	int GetRemaining()      const { return m_values[1]; }
	int GetResetAfter()     const { return m_values[2]; }
	int GetMaxConcurrency() const { return m_values[3]; }
};
typedef const cSessionStartLimit* hSessionStartLimit;

class cGatewayInfo final {
private:
	hJsonError         error               = nullptr;
	hSessionStartLimit session_start_limit = nullptr;
	int                shards              = 0;
	json::string       url;
	
public:
	/* Get gateway info - throws exceptions */
	cGatewayInfo(const json::value& v);
	cGatewayInfo(const cGatewayInfo&);
	cGatewayInfo(cGatewayInfo&&);
	~cGatewayInfo();
	
	cGatewayInfo& operator=(cGatewayInfo);
	
	hJsonError         GetError()             const { return error;               }
	hSessionStartLimit GetSessionStartLimit() const { return session_start_limit; }
	const char*        GetUrl()               const { return url.c_str();         }
	int                GetShards()            const { return shards;              }
};
typedef const cGatewayInfo* hGatewayInfo;

#endif /* _GREEKBOT_GATEWAYINFO_H_ */
