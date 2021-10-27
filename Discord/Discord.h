#pragma once
#ifndef _GREEKBOT_DISCORD_H_
#define _GREEKBOT_DISCORD_H_
#include "Types.h"

#define _STR(x) #x
#define STR(x) _STR(x)

#define DISCORD_API_VERSION     9
#define DISCORD_API_HOST        "discord.com"
#define DISCORD_API_ENDPOINT    "/api/v" STR(DISCORD_API_VERSION)
#define DISCORD_API_GATEWAY     DISCORD_API_ENDPOINT "/gateway"
#define DISCORD_API_GATEWAY_BOT DISCORD_API_GATEWAY  "/bot"

enum eDiscordErrorType {
	DISCORD_ERROR_NO_ERROR,
	DISCORD_ERROR_GENERIC,
	DISCORD_ERROR_HTTP,
	DISCORD_ERROR_JSON
};

class cDiscordError final {
private:
	class _error final {
	private:
		int   m_code;
		char* m_message;

	public:
		/* Constructors */
		_error() : m_code(0), m_message(nullptr) {}
		explicit _error(const json::value& v) : m_code(v.at("code").as_int64()) {
			auto& s = v.at("message").as_string();
			m_message = new char[s.size() + 1];
			strcpy(m_message, s.c_str());
		}
		_error(const _error& o) : m_code(o.m_code) {
			if (o.m_message) {
				m_message = new char[strlen(o.m_message) + 1];
				strcpy(m_message, o.m_message);
				return;
			}
			m_message = nullptr;
		}
		_error(_error&& o) noexcept : m_code(o.m_code), m_message(o.m_message) {
			o.m_code = 0;
			o.m_message = nullptr;
		}
		~_error() { delete[] m_message; }
		/* Assignment */
		_error& operator=(_error o) {
			m_code = o.m_code;
			char* temp = m_message;
			m_message = o.m_message;
			o.m_message = temp;
			return *this;
		}
		/* Attributes */
		[[nodiscard]]
		int GetCode() const { return m_code; }
		[[nodiscard]]
		const char* GetMessage() const { return m_message; }
	};

	eDiscordErrorType m_type;
	_error m_error;

	explicit cDiscordError(eDiscordErrorType t) : m_type(t) {}
	cDiscordError(eDiscordErrorType t, const json::value& v) : m_type(t) {
		try {
			new (&m_error) _error(v);
		}
		catch (...) {
			m_type = DISCORD_ERROR_GENERIC;
			new (&m_error) _error();
		}
	}

public:
	cDiscordError() : m_type(DISCORD_ERROR_NO_ERROR) {}

	cDiscordError(const cDiscordError& o) = default;
	cDiscordError(cDiscordError&& o) noexcept : m_type(o.m_type), m_error(std::move(o.m_error)) {}

	cDiscordError& operator=(const cDiscordError& o) {
		m_type = o.m_type;
		m_error = o.m_error;
		return *this;
	}
	cDiscordError& operator=(cDiscordError&& o) noexcept {
		m_type = o.m_type;
		m_error = std::move(o.m_error);
		return *this;
	}

	explicit operator bool() const { return m_type != DISCORD_ERROR_NO_ERROR; }

	template<eDiscordErrorType t, typename = std::enable_if_t<t == DISCORD_ERROR_NO_ERROR || t == DISCORD_ERROR_GENERIC>>
	static cDiscordError MakeError() { return cDiscordError(t); }
	template<eDiscordErrorType t, typename = std::enable_if_t<t != DISCORD_ERROR_NO_ERROR && t != DISCORD_ERROR_GENERIC>>
	static cDiscordError MakeError(const json::value& v) { return { t, v }; }

	[[nodiscard]]
	eDiscordErrorType GetType() const { return m_type; }
	[[nodiscard]]
	const _error* Get() const { return m_error.GetMessage() ? &m_error : nullptr; }
};
typedef hHandle<cDiscordError>     hDiscordError;
typedef chHandle<cDiscordError>   chDiscordError;
typedef uhHandle<cDiscordError>   uhDiscordError;
typedef uchHandle<cDiscordError> uchDiscordError;
typedef shHandle<cDiscordError>   shDiscordError;
typedef schHandle<cDiscordError> schDiscordError;

/* A helper class that performs HTTP requests to the API endpoint */
class cDiscord final {
private:
	cDiscord() = default;

public:
	static unsigned int GetHttpsRequest(const char* path, const char* auth, std::string& response, cDiscordError& error);
};

#endif /* _GREEKBOT_DISCORD_H_*/
