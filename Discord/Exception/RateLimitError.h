#ifndef DISCORD_RATELIMITERROR_H
#define DISCORD_RATELIMITERROR_H
#include "Common.h"
#include <chrono>
#include <stdexcept>

class xRateLimitError : public std::runtime_error {
	std::chrono::milliseconds m_retry_after{};
	bool                      m_global{};

public:
	xRateLimitError(const char* what_arg, const boost::json::object* obj = nullptr);
	xRateLimitError(const std::string& what_arg, const boost::json::object* obj = nullptr);

	auto retry_after() const noexcept { return m_retry_after; }
	auto      global() const noexcept { return m_global;      }
};
#endif //DISCORD_RATELIMITERROR_H
