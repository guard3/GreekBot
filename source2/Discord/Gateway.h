#pragma once
#ifndef _GREEKBOT_GATEWAY_H_
#define _GREEKBOT_GATEWAY_H_
#include "JsonError.h"

class cSessionStartLimit final {
private:
	int m_total, m_remaining, m_reset_after, m_max_concurrency;
	
public:
	/* Construct a session_start_object from a gateway JSON object */
	cSessionStartLimit(const json::value&);
	
	/* Default copy/move constructors */
	cSessionStartLimit(const cSessionStartLimit& ) = default;
	cSessionStartLimit(      cSessionStartLimit&&) = default;
	
	/* Attributes */
	int GetTotal()          const { return m_total;           }
	int GetRemaining()      const { return m_remaining;       }
	int GetResetAfter()     const { return m_reset_after;     }
	int GetMaxConcurrency() const { return m_max_concurrency; }
	
};

class cGateway final {
private:
	/* The gateway JSON object */
	json::value m_json;
	
	/* The error object */
	const cJsonError* m_errorObject = nullptr;
	
	/* Attributes */
	const char* m_url = nullptr;
	int m_shards = 0;
	const cSessionStartLimit* m_session_start_limit = nullptr;
	
public:
	/* Get a gateway object with provided HTTP Authorization header */
	cGateway(const char* auth);
	~cGateway();
	
	/* Attributes */
	const cJsonError* GetError() const { return m_errorObject; }
	const char* GetUrl() const { return m_url; }
	int GetShards() const { return m_shards; }
	const cSessionStartLimit& GetSessionStartLimit() { return *m_session_start_limit; }
	
	/* Convert to bool */
	operator bool() { return !GetError(); }
};

#endif /* _GREEKBOT_GATEWAY_H_*/
