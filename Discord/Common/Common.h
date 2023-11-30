#ifndef GREEKBOT_COMMON_H
#define GREEKBOT_COMMON_H
#include "kwarg.h"
#include "Ptr.h"
#include <chrono>
#include <compare>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <fmt/format.h>

#define STR_(x) #x
#define STR(x) STR_(x)

#define DISCORD_API_VERSION     10
#define DISCORD_API_HOST        "discord.com"
#define DISCORD_API_ENDPOINT    "/api/v" STR(DISCORD_API_VERSION)

/* Boost Json forward declarations */
namespace boost::json {
	class value;
	class object;

	template<typename>
	struct value_to_tag;
	struct value_from_tag;
}
namespace json = boost::json;

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
/* ========== Discord snowflake ===================================================================================== */
class cSnowflake final {
private:
	char     m_str[20]; // The snowflake as a string
	uint32_t m_len;     // The length of the string
	uint64_t m_int;     // The snowflake as a 64-bit integer
	
public:
	cSnowflake() noexcept : m_int(0), m_len(1), m_str("0") {}
	cSnowflake(uint64_t i) noexcept;
	cSnowflake(std::string_view s);
	template<typename T> requires std::constructible_from<std::string_view, T&&>
	cSnowflake(T&& t) : cSnowflake(std::string_view{ std::forward<T>(t) }) {}

	auto operator<=>(const cSnowflake& o) const noexcept { return m_int <=> o.m_int; }
	bool operator== (const cSnowflake& o) const noexcept { return m_int ==  o.m_int; }

	/* Attributes */
	std::string_view ToString() const noexcept { return { m_str, m_len }; }
	uint64_t         ToInt()    const noexcept { return m_int; }
	
	/* Snowflake components - https://discord.com/developers/docs/reference#snowflakes */
	auto  GetInternalWorkerId() const noexcept { return static_cast<std::uint8_t >(0x01F & (m_int >> 17)); }
	auto GetInternalProcessId() const noexcept { return static_cast<std::uint8_t >(0x01F & (m_int >> 12)); }
	auto         GetIncrement() const noexcept { return static_cast<std::uint16_t>(0xFFF &  m_int       ); }
	/* Resource timestamp */
	auto GetTimestamp() const noexcept {
		using namespace std::chrono;
		using namespace std::chrono_literals;
		return sys_days(2015y/1/1) + milliseconds(m_int >> 22);
	}
};
typedef   hHandle<cSnowflake>   hSnowflake;
typedef  chHandle<cSnowflake>  chSnowflake;
typedef  uhHandle<cSnowflake>  uhSnowflake;
typedef uchHandle<cSnowflake> uchSnowflake;

cSnowflake tag_invoke(boost::json::value_to_tag<cSnowflake>, const boost::json::value&);

template<>
struct fmt::formatter<cSnowflake> : fmt::formatter<std::string_view> {
	auto format(const cSnowflake& sf, auto& ctx) const {
		return fmt::formatter<std::string_view>::format(sf.ToString(), ctx);
	}
};

/* ========== Color ========== */
class cColor final {
private:
	int32_t m_value;

public:
	static inline constexpr int32_t NO_COLOR = 0;
	cColor() noexcept : m_value(NO_COLOR) {}
	cColor(int32_t v) noexcept : m_value(v) {}

	uint8_t GetRed()   const noexcept { return (m_value >> 16) & 0xFF; }
	uint8_t GetGreen() const noexcept { return (m_value >>  8) & 0xFF; }
	uint8_t GetBlue()  const noexcept { return  m_value        & 0xFF; }

	int ToInt() const noexcept { return m_value; }
	operator int() const noexcept { return m_value; }
	operator bool() const noexcept { return m_value != NO_COLOR; }
	bool operator!() const noexcept { return m_value == NO_COLOR; }
};

cColor tag_invoke(boost::json::value_to_tag<cColor>, const boost::json::value&);

KW_DECLARE(color, cColor)
KW_DECLARE(title, std::string)
KW_DECLARE(description, std::string)
KW_DECLARE(url, std::string)
KW_DECLARE(icon_url, std::string)
KW_DECLARE(timestamp, std::string)
#endif // GREEKBOT_COMMON_H