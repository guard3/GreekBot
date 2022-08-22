#pragma once
#ifndef _GREEKBOT_NET_H_
#define _GREEKBOT_NET_H_
#include "Common.h"
#include <stdexcept>

class xSystemError : public std::runtime_error {
public:
	int m_code;

public:
	explicit xSystemError(const json::object&);
	explicit xSystemError(const json::value&);
	explicit xSystemError(const std::string& what, int code = 0) : std::runtime_error(what), m_code(0) {}
	explicit xSystemError(const char*        what, int code = 0) : std::runtime_error(what), m_code(0) {}
	explicit xSystemError(const boost::system::system_error&);
	xSystemError(const xSystemError&) = default;
	xSystemError(xSystemError&&)      = default;

	xSystemError& operator=(const xSystemError&) = default;
	xSystemError& operator=(xSystemError&&)      = default;

	int code() const noexcept { return m_code; }
};

class xDiscordError : public xSystemError {
private:
	std::string m_errors;

public:
	explicit xDiscordError(const json::object&);
	explicit xDiscordError(const json::value&);
	xDiscordError(const xDiscordError&) = default;
	xDiscordError(xDiscordError&&)      = default;

	xDiscordError& operator=(const xDiscordError&) = default;
	xDiscordError& operator=(xDiscordError&&)      = default;

	const char* errors() const noexcept { return m_errors.c_str(); }
};

class cDiscord final {
private:
	cDiscord() = default;
	
public:
	static json::value HttpGet(const std::string& path, const std::string& auth);
	static void        HttpPost(const std::string& path, const std::string& auth, const json::object& obj);
	static json::value HttpPatch(const std::string& path, const std::string& auth, const json::object& obj);
	static json::value HttpPut(const std::string& path, const std::string& auth);
	static json::value HttpPut(const std::string& path, const std::string& auth, const json::object& obj);
	static json::value HttpDelete(const std::string& path, const std::string& auth);
};
#endif /* _GREEKBOT_NET_H_ */