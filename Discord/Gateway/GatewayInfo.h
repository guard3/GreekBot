#pragma once
#ifndef _GREEKBOT_GATEWAYINFO_H_
#define _GREEKBOT_GATEWAYINFO_H_
#include "Common.h"

class cSessionStartLimit final {
private:
	int m_values[4];
	
public:
	explicit cSessionStartLimit(const json::object&);
	explicit cSessionStartLimit(const json::value&);

	int GetTotal()          const noexcept { return m_values[0]; }
	int GetRemaining()      const noexcept { return m_values[1]; }
	int GetResetAfter()     const noexcept { return m_values[2]; }
	int GetMaxConcurrency() const noexcept { return m_values[3]; }
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
	explicit cGatewayInfo(const json::object&);
	explicit cGatewayInfo(const json::value&);

	const std::string&        GetUrl()               const noexcept { return url;                 }
	int                       GetShards()            const noexcept { return shards;              }
	const cSessionStartLimit& GetSessionStartLimit() const noexcept { return session_start_limit; }
};
typedef   hHandle<cGatewayInfo>   hGatewayInfo;
typedef  chHandle<cGatewayInfo>  chGatewayInfo;
typedef  uhHandle<cGatewayInfo>  uhGatewayInfo;
typedef uchHandle<cGatewayInfo> uchGatewayInfo;
typedef  shHandle<cGatewayInfo>  shGatewayInfo;
typedef schHandle<cGatewayInfo> schGatewayInfo;
#endif /* _GREEKBOT_GATEWAYINFO_H_ */