#ifndef GREEKBOT_COMMON_H
#define GREEKBOT_COMMON_H
#include <compare>
#include <memory>
#include <chrono>
#include <random>
#include <string>
#include <optional>
#include <cinttypes>
#include <cstring>
#include "Exception.h"
#include "Utils.h"
#include "Kwarg.h"
#include "Ptr.h"

namespace chrono = std::chrono;
using namespace std::literals::chrono_literals;

#define STR_(x) #x
#define STR(x) STR_(x)

#define DISCORD_API_VERSION     10
#define DISCORD_API_HOST        "discord.com"
#define DISCORD_API_ENDPOINT    "/api/v" STR(DISCORD_API_VERSION)

#define DISCORD_IMAGE_BASE_URL "https://cdn.discordapp.com/"

using namespace std::literals;

/* Boost Json forward declarations */
namespace boost::json {
	class value;
	class object;
	class string;
}
namespace json = boost::json;

/* Declare itoa variants if they're not provided by the standard library */
char* utoa (unsigned, char*, int);
char* ultoa (unsigned long, char*, int);
char* ulltoa (unsigned long long, char*, int);

/* ========== Handle types ========== */
template<typename T> // handle
using   hHandle = cPtr<T>;
template<typename T> // const handle
using  chHandle = cPtr<const T>;
template<typename T> // unique handle
using  uhHandle = std::unique_ptr<T>;
template<typename T> // unique const handle
using uchHandle = std::unique_ptr<const T>;
template<typename T> // shared handle
using  shHandle = std::shared_ptr<T>;
template<typename T> // shared const handle
using schHandle = std::shared_ptr<const T>;

/* ========== Handle creation functions ========== */
class cHandle final {
private:
	cHandle() = default;

public:
	template<typename T, typename... Args>
	static uhHandle<T> MakeUnique(Args&&... args) { return std::make_unique<T>(std::forward<Args>(args)...); }
	template<typename T, typename... Args>
	static shHandle<T> MakeShared(Args&&... args) { return std::make_shared<T>(std::forward<Args>(args)...); }
	template<typename T, typename... Args>
	static uchHandle<T> MakeUniqueConst(Args&&... args) { return MakeUnique<T>(std::forward<Args>(args)...); }
	template<typename T, typename... Args>
	static schHandle<T> MakeSharedConst(Args&&... args) { return MakeShared<T>(std::forward<Args>(args)...); }

	template<typename T, typename... Args>
	static uhHandle<T> MakeUniqueNoEx(Args&&... args) {
		try {
			return MakeUnique<T>(std::forward<Args>(args)...);
		}
		catch (...) {
			return {};
		}
	}
	template<typename T, typename... Args>
	static shHandle<T> MakeSharedNoEx(Args&&... args) {
		try {
			return MakeShared<T>(std::forward<Args>(args)...);
		}
		catch (...) {
			return {};
		}
	}
	template<typename T, typename... Args>
	static uchHandle<T> MakeUniqueConstNoEx(Args&&... args) { return MakeUniqueNoEx<T>(std::forward<Args>(args)...); }
	template<typename T, typename... Args>
	static schHandle<T> MakeSharedConstNoEx(Args&&... args) { return MakeSharedNoEx<T>(std::forward<Args>(args)...); }
};
/* ========== Discord time point ==================================================================================== */
class cDiscordClock;
template<typename Duration>
using tDiscordTime = chrono::time_point<cDiscordClock, Duration>;
/* ========== Custom clock starting from Discord Epoch (1-1-2015) =================================================== */
class cDiscordClock final {
public:
	/* Necessary attributes to meet requirements for 'Clock' */
	typedef chrono::milliseconds duration;
	typedef duration::rep rep;
	typedef duration::period period;
	typedef chrono::time_point<cDiscordClock> time_point;
	static inline constexpr bool is_steady = false;
	/* Converting to and from std::chrono::system_clock time_points similar to std::chrono::utc_clock */
	template<typename Duration>
	static time_point from_sys(const chrono::sys_time<Duration>& p) noexcept {
		using namespace chrono;
		return time_point(duration_cast<milliseconds>(p.time_since_epoch()) - 1420070400000ms);
	}
	template<typename Duration>
	static auto to_sys(const tDiscordTime<Duration>& p) noexcept {
		using namespace chrono;
		typedef std::common_type_t<Duration, system_clock::duration> tCommon;
		return sys_time<tCommon>(duration_cast<tCommon>(p.time_since_epoch() + 1420070400000ms));
	}
	/* Get current time starting from Discord Epoch */
	static time_point now() noexcept { return from_sys(chrono::system_clock::now()); }
};
/* ========== Discord time point family ============================================================================= */
using tDiscordDays         = tDiscordTime<chrono::days>;
using tDiscordSeconds      = tDiscordTime<chrono::seconds>;
using tDiscordMilliseconds = tDiscordTime<chrono::milliseconds>;
/* ========== Discord snowflake ===================================================================================== */
class cSnowflake final {
private:
	char     m_str[24]; // The snowflake as a string
	uint64_t m_int;     // The snowflake as a 64-bit integer
	
public:
	cSnowflake() noexcept : m_int(0), m_str("0") {}
	cSnowflake(std::integral auto i) noexcept : m_int(static_cast<uint64_t>(i)) { ulltoa(static_cast<uint64_t>(i), m_str, 10); }
	cSnowflake(const char* s) : m_int(cUtils::ParseInt<uint64_t>(s)) { strcpy(m_str, s); }
	cSnowflake(const std::string& s) : cSnowflake(s.c_str()) {}
	cSnowflake(std::nullptr_t) = delete;
	explicit cSnowflake(const json::value&);
	explicit cSnowflake(const json::string&);

	auto operator<=>(const cSnowflake& o) const noexcept { return m_int <=> o.m_int; }
	bool operator== (const cSnowflake& o) const noexcept { return m_int ==  o.m_int; }

	/* Attributes */
	const char* ToString() const noexcept { return m_str; }
	uint64_t       ToInt() const noexcept { return m_int; }
	
	/* Snowflake components - https://discord.com/developers/docs/reference#snowflakes */
	tDiscordMilliseconds GetTimestamp() const noexcept { return tDiscordMilliseconds(chrono::milliseconds(m_int >> 22)); }
	int           GetInternalWorkerID() const noexcept { return (int)((m_int >> 17) & 0x01F); }
	int          GetInternalProcessID() const noexcept { return (int)((m_int >> 12) & 0x01F); }
	int                  GetIncrement() const noexcept { return (int) (m_int        & 0xFFF); }
};
typedef   hHandle<cSnowflake>   hSnowflake; // handle
typedef  chHandle<cSnowflake>  chSnowflake; // const handle
typedef  uhHandle<cSnowflake>  uhSnowflake; // unique handle
typedef uchHandle<cSnowflake> uchSnowflake; // unique const handle
typedef  shHandle<cSnowflake>  shSnowflake; // shared handle
typedef schHandle<cSnowflake> schSnowflake; // shared const handle

/* ========== Color ========== */
class cColor final {
private:
	int32_t m_value;

public:
	static inline constexpr int32_t NO_COLOR = 0;
	cColor() noexcept : m_value(NO_COLOR) {}
	cColor(int32_t v) noexcept : m_value(v) {}
	cColor(const json::value&);

	uint8_t GetRed()   const noexcept { return (m_value >> 16) & 0xFF; }
	uint8_t GetGreen() const noexcept { return (m_value >>  8) & 0xFF; }
	uint8_t GetBlue()  const noexcept { return  m_value        & 0xFF; }

	int ToInt() const noexcept { return m_value; }
	operator int() const noexcept { return m_value; }
	operator bool() const noexcept { return m_value != NO_COLOR; }
	bool operator!() const noexcept { return m_value == NO_COLOR; }
};

KW_DECLARE(color, KW_COLOR, cColor)
KW_DECLARE(title, KW_TITLE, std::string)
KW_DECLARE(description, KW_DESCRIPTION, std::string)
KW_DECLARE(url, KW_URL, std::string)
KW_DECLARE(icon_url, KW_ICON_URL, std::string)
KW_DECLARE(timestamp, KW_TIMESTAMP, std::string)
#endif // GREEKBOT_COMMON_H