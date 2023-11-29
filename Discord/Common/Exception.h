#ifndef GREEKBOT_EXCEPTION_H
#define GREEKBOT_EXCEPTION_H
#include <stdexcept>
#include <string>
#include <chrono>

namespace boost::json {
	class object;
	class value;
}
namespace json = boost::json;

class xSystemError : public std::runtime_error {
public:
	int m_code;

public:
	explicit xSystemError(const std::string& what, int code = 0) : std::runtime_error(what), m_code(code) {}
	explicit xSystemError(const char*        what, int code = 0) : std::runtime_error(what), m_code(code) {}

	int code() const noexcept { return m_code; }
};

class xDiscordError : public xSystemError {
private:
	std::string m_errors;

public:
	explicit xDiscordError(const json::object&);
	explicit xDiscordError(const json::value&);

	const char* errors() const noexcept { return m_errors.c_str(); }
};

class xRateLimitError : public std::runtime_error {
private:
	std::chrono::milliseconds m_retry_after;
	bool                      m_global;

public:
	explicit xRateLimitError(const json::object&);
	explicit xRateLimitError(const json::value&);

	auto retry_after() const noexcept { return m_retry_after; }
	bool      global() const noexcept { return m_global;      }
};
#endif //GREEKBOT_EXCEPTION_H