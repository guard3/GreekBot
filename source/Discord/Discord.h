#pragma once
/* Stupid WIN32 */
#undef GetMessage

/* Includes */
#include <rapidjson/document.h>
#include <curl/curl.h>
#include "JsonError.h"

/* Helper macro for int to string */
#define _STR(x) #x
#define STR(x) _STR(x)

/* Discord API version to be used */
#define DISCORD_API_VERSION 9

#define DISCORD_API_ENDPOINT "https://discord.com/api/v" STR(DISCORD_API_VERSION)
#define DISCORD_API_GATEWAY DISCORD_API_ENDPOINT "/gateway"
#define DISCORD_API_GATEWAY_BOT DISCORD_API_GATEWAY "/bot"

class cDiscord final {
private:
	cDiscord() {}

public:
	static CURLcode GetGateway(const char* token, rapidjson::Document& document);
};

class cSessionStartLimit final {
	friend class cGateway;

private:
	int m_total = 0, m_remaining = 0, m_reset_after = 0, m_max_concurrency = 0;

	cSessionStartLimit() {}

public:
	int GetTotal()          const { return m_total;           }
	int GetRemaining()      const { return m_remaining;       }
	int GetResetAfter()     const { return m_reset_after;     }
	int GetMaxConcurrency() const { return m_max_concurrency; }
};

class cGateway final {
private:
	/* The JSON document from which data is read */
	rapidjson::Document m_document;
	
	cJsonError* m_pError = nullptr;

	const char* m_url = nullptr;
	int m_shards = 0;
	cSessionStartLimit m_session_start_limit;

public:
	/* Constructors */
	cGateway(const char* token);
	~cGateway() { delete m_pError; }

	/* Attributes */
	const cJsonError* GetError() { return m_pError; }
	const char* GetUrl() { return m_url; }
	int GetShards() { return m_shards; }
	const cSessionStartLimit& GetSessionStartLimit() { return m_session_start_limit; }

	operator bool() { return !m_pError; }
};