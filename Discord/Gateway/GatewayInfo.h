#pragma once
#ifndef _GREEKBOT_GATEWAYINFO_H_
#define _GREEKBOT_GATEWAYINFO_H_
#include "Error.h"

class cSessionStartLimit final {
private:
	int m_values[4];
	
public:
	/* Construct from a JSON - throws exceptions */
	explicit cSessionStartLimit(const json::value& v) : m_values {
		static_cast<int>(v.at("total"          ).as_int64()),
		static_cast<int>(v.at("remaining"      ).as_int64()),
		static_cast<int>(v.at("reset_after"    ).as_int64()),
		static_cast<int>(v.at("max_concurrency").as_int64())
	} {}
	
	/* Attributes */
	[[nodiscard]] int GetTotal()          const { return m_values[0]; }
	[[nodiscard]] int GetRemaining()      const { return m_values[1]; }
	[[nodiscard]] int GetResetAfter()     const { return m_values[2]; }
	[[nodiscard]] int GetMaxConcurrency() const { return m_values[3]; }
};
typedef   hHandle<cSessionStartLimit>   hSessionStartLimit;
typedef  chHandle<cSessionStartLimit>  chSessionStartLimit;
typedef  uhHandle<cSessionStartLimit>  uhSessionStartLimit;
typedef uchHandle<cSessionStartLimit> uchSessionStartLimit;
typedef  shHandle<cSessionStartLimit>  shSessionStartLimit;
typedef schHandle<cSessionStartLimit> schSessionStartLimit;

class cGatewayInfo final {
private:
	std::string        url;
	int                shards;
	cSessionStartLimit session_start_limit;
	
public:
	/* Construct from a JSON - throws exceptions */
	explicit cGatewayInfo(const json::value& v) : url(v.at("url").as_string().c_str()), shards(static_cast<int>(v.at("shards").as_int64())), session_start_limit(v.at("session_start_limit")) {}

	/* Attributes */
	[[nodiscard]] const char         *GetUrl()               const { return url.c_str();          }
	[[nodiscard]] int                 GetShards()            const { return shards;               }
	[[nodiscard]] chSessionStartLimit GetSessionStartLimit() const { return &session_start_limit; }
};
typedef   hHandle<cGatewayInfo>   hGatewayInfo;
typedef  chHandle<cGatewayInfo>  chGatewayInfo;
typedef  uhHandle<cGatewayInfo>  uhGatewayInfo;
typedef uchHandle<cGatewayInfo> uchGatewayInfo;
typedef  shHandle<cGatewayInfo>  shGatewayInfo;
typedef schHandle<cGatewayInfo> schGatewayInfo;
#endif /* _GREEKBOT_GATEWAYINFO_H_ */