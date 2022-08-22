#pragma once
#ifndef _GREEKBOT_COMMON_H_
#define _GREEKBOT_COMMON_H_
#include <memory>
#include <type_traits>
#include <random>
#include <string>
#include <cinttypes>
#include <cstring>

#define _STR(x) #x
#define STR(x) _STR(x)

#define DISCORD_API_VERSION     9
#define DISCORD_API_HOST        "discord.com"
#define DISCORD_API_ENDPOINT    "/api/v" STR(DISCORD_API_VERSION)

#define DISCORD_IMAGE_BASE_URL "https://cdn.discordapp.com/"

using namespace std::literals;

/* Boost Json forward declarations */
namespace boost {
	namespace json {
		class value;
		class object;
	}
	namespace system {
		class system_error;
	}
}
namespace json = boost::json;

/* ========== Handle types ========== */
template<typename T> // handle
using   hHandle = T*;
template<typename T> // const handle
using  chHandle = const T*; // ch: const handle
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

	template<typename T> struct is_handle              : std::false_type {};
	template<typename T> struct is_handle<T*>          : std::true_type  {};
	template<typename T> struct is_handle<uhHandle<T>> : std::true_type  {};
	template<typename T> struct is_handle<shHandle<T>> : std::true_type  {};

	template<typename T> struct remove_h { typedef T Type; };
	template<typename T> struct remove_h<hHandle<T>> { typedef T Type; };
	template<typename T> struct remove_h<uhHandle<T>> { typedef T Type; };
	template<typename T> struct remove_h<shHandle<T>> { typedef T Type; };

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

	template<typename T>
	static inline constexpr bool IsHandleType = is_handle<T>::value;
	template<typename T>
	using RemoveHandle = typename remove_h<std::remove_cv_t<T>>::Type;
	template<typename T>
	using RemoveHandleC = std::remove_const_t<RemoveHandle<T>>;
	template<typename T>
	using RemoveHandleV = std::remove_volatile_t<RemoveHandle<T>>;
	template<typename T>
	using RemoveHandleCV = std::remove_cv_t<RemoveHandle<T>>;
};

/* ========== Discord snowflake ========== */
class cSnowflake final {
private:
	char     m_str[24]; // The snowflake as a string
	uint64_t m_int;     // The snowflake as a 64-bit integer
	
public:
	cSnowflake() noexcept : m_int(0), m_str("0") {}
	cSnowflake(uint64_t i) noexcept : m_int(i) { sprintf(m_str, "%" PRIu64, i); }
	cSnowflake(const char* str) noexcept : m_int(strtoull(str, nullptr, 10)) { strcpy(m_str, str); }
	cSnowflake(const json::value& v);

	bool operator==(const cSnowflake& o) const { return m_int == o.m_int; }
	bool operator!=(const cSnowflake& o) const { return m_int != o.m_int; }
	bool operator< (const cSnowflake& o) const { return m_int <  o.m_int; }
	bool operator> (const cSnowflake& o) const { return m_int >  o.m_int; }
	bool operator<=(const cSnowflake& o) const { return m_int <= o.m_int; }
	bool operator>=(const cSnowflake& o) const { return m_int >= o.m_int; }

	/* Attributes */
	const char *ToString() const { return m_str; }
	uint64_t    ToInt()    const { return m_int; }
	
	/* Snowflake components - https://discord.com/developers/docs/reference#snowflakes */
	time_t GetTimestamp()         const { return (m_int >> 22) / 1000 + 1420070400; }
	int    GetInternalWorkerID()  const { return (m_int >> 17) & 0x1F;              }
	int    GetInternalProcessID() const { return (m_int >> 12) & 0x1F;              }
	int    GetIncrement()         const { return  m_int        & 0xFFF;             }
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
	cColor() : m_value(NO_COLOR) {}
	cColor(int32_t v) : m_value(v) {}
	cColor(const json::value&);

	uint8_t GetRed()   const { return (m_value >> 16) & 0xFF; }
	uint8_t GetGreen() const { return (m_value >>  8) & 0xFF; }
	uint8_t GetBlue()  const { return  m_value        & 0xFF; }

	int ToInt() const { return m_value; }
	operator int() const { return m_value; }
	operator bool() const { return m_value != NO_COLOR; }
	bool operator!() const { return m_value == NO_COLOR; }
};

namespace hidden {
	template<typename T, typename = void> struct d;
	template<typename T> struct d<T, std::enable_if_t<std::is_integral_v<T>>>       { typedef std::uniform_int_distribution<T>  type; };
	template<typename T> struct d<T, std::enable_if_t<std::is_floating_point_v<T>>> { typedef std::uniform_real_distribution<T> type; };
	template<typename T> using distribution = typename d<T>::type;
	template<typename T> using range = typename distribution<T>::param_type;
}

/* A helper class with various useful functions */
class cUtils final {
private:
	/* Static random generators */
	static std::random_device ms_rd;
	static std::mt19937       ms_gen;
	static std::mt19937_64    ms_gen64;

	template<typename T>
	static auto resolve_fargs(T&& arg) { return arg; }
	static auto resolve_fargs(std::string&& str) { return str.c_str(); }
	static auto resolve_fargs(std::string& str) { return str.c_str(); }
	static auto resolve_fargs(const std::string& str) { return str.c_str(); }

	static void print(FILE*, const char*, char, const char*, ...);
	static std::string format(const char*, ...);
	/* Private constructor */
	cUtils() = default;

public:
	/* Random functions */
	template<typename T1, typename T2, typename R = std::common_type_t<T1, T2>>
	static R Random(T1 a, T2 b) {
		/* Static uniform distribution */
		static hidden::distribution<R> dist;
		/* Generate random number */
		if constexpr(sizeof(R) < 8)
			return dist(ms_gen,   hidden::range<R>(a, b));
		else
			return dist(ms_gen64, hidden::range<R>(a, b));
	}
	/* Logger functions */
	template<char nl = '\n', typename... Args>
	static void PrintErr(const char* fmt, Args&&... args) {
		print(stderr, "[ERR] ", nl, fmt, resolve_fargs(std::forward<Args>(args))...);
	}
	template<char nl = '\n', typename... Args>
	static void PrintLog(const char* fmt, Args&&... args) {
		print(stdout, "[LOG] ", nl, fmt, resolve_fargs(std::forward<Args>(args))...);
	}
	template<char nl = '\n'>
	static void PrintErr(const std::string& str) { PrintErr<nl>(str.c_str()); }
	template<char nl = '\n'>
	static void PrintLog(const std::string& str) { PrintLog<nl>(str.c_str()); }
	/* C style formatting for std::string */
	template<typename... Args>
	static std::string Format(const char* fmt, Args&&... args) {
		return format(fmt, resolve_fargs(std::forward<Args>(args))...);
	}
	/* Resolving the OS we're running on */
	static const char* GetOS();
};

#endif /* _GREEKBOT_COMMON_H_ */
