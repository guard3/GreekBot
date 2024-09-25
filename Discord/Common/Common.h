#ifndef DISCORD_COMMON_H
#define DISCORD_COMMON_H
#include "Ptr.h"
#include <chrono>
#include <compare>
#include <cstdint>
#include <format>
#include <memory>
#include <string>
#include <string_view>

#define DISCORD_STR_(x) #x
#define DISCORD_STR(x) DISCORD_STR_(x)

#define DISCORD_API_VERSION     10
#define DISCORD_API_VERSION_STR DISCORD_STR(DISCORD_API_VERSION)
#define DISCORD_API_HOST        "discord.com"
#define DISCORD_API_ENDPOINT    "/api/v" DISCORD_API_VERSION_STR

/* ========== Boost/JSON forward declarations ======================================================================= */
namespace boost::json {
	class value;
	class object;

	template<typename>
	struct value_to_tag;
	struct value_from_tag;

	template<typename>
	struct is_variant_like;
}

/* ========== Handle types ========================================================================================== */
template<typename T> // handle
using   hHandle = cPtr<T>;
template<typename T> // const handle
using  chHandle = cPtr<const T>;
template<typename T> // unique handle
using  uhHandle = std::unique_ptr<T>;
template<typename T> // unique const handle
using uchHandle = std::unique_ptr<const T>;

/* ========== Discord snowflake ===================================================================================== */
class cSnowflake final {
	char          m_str[20]; // The snowflake as a string
	std::uint32_t m_len;     // The length of the string
	std::uint64_t m_int;     // The snowflake as a 64-bit integer
	
public:
	cSnowflake() noexcept : m_int(0), m_len(1), m_str("0") {}
	cSnowflake(std::uint64_t i) noexcept;
	cSnowflake(std::string_view s);
	template<typename T> requires std::constructible_from<std::string_view, T&&>
	cSnowflake(T&& t) : cSnowflake(std::string_view{ std::forward<T>(t) }) {}

	auto operator<=>(const cSnowflake& o) const noexcept { return m_int <=> o.m_int; }
	bool operator== (const cSnowflake& o) const noexcept { return m_int ==  o.m_int; }

	/* Attributes */
	std::string_view ToString() const noexcept { return { m_str, m_len }; }
	std::uint64_t       ToInt() const noexcept { return m_int;            }
	
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
cSnowflake
tag_invoke(boost::json::value_to_tag<cSnowflake>, const boost::json::value&);
void
tag_invoke(boost::json::value_from_tag, boost::json::value&, const cSnowflake&);
/* ========== Make cSnowflake formattable =========================================================================== */
template<typename CharT>
struct std::formatter<cSnowflake, CharT> : std::formatter<std::string_view, CharT> {
	template<typename OutputIt>
	OutputIt format(const cSnowflake& sf, std::basic_format_context<OutputIt, CharT>& ctx) const {
		return std::formatter<std::string_view, CharT>::format(sf.ToString(), ctx);
	}
};
/* ========== Color ================================================================================================= */
class cColor final {
	std::int32_t m_value;

public:
	constexpr cColor() noexcept : m_value(0) {}
	constexpr cColor(std::int32_t v) noexcept : m_value(v) {}

	std::uint8_t GetRed()   const noexcept { return (m_value >> 16) & 0xFF; }
	std::uint8_t GetGreen() const noexcept { return (m_value >>  8) & 0xFF; }
	std::uint8_t GetBlue()  const noexcept { return  m_value        & 0xFF; }

	std::int32_t ToInt() const noexcept { return m_value; }
	operator int() const noexcept { return m_value; }
	operator bool() const noexcept { return m_value; }
	bool operator!() const noexcept { return !m_value; }
};
using   hColor =   hHandle<cColor>;
using  chColor =  chHandle<cColor>;
using  uhColor =  uhHandle<cColor>;
using uchColor = uchHandle<cColor>;
cColor
tag_invoke(boost::json::value_to_tag<cColor>, const boost::json::value&);
void
tag_invoke(boost::json::value_from_tag, boost::json::value&, cColor);
#endif /* DISCORD_COMMON_H */