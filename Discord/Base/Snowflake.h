#ifndef DISCORD_SNOWFLAKE_H
#define DISCORD_SNOWFLAKE_H
#include "SnowflakeFwd.h"
#include <charconv>
#include <chrono>
#include <string_view>
#include <type_traits>
/* ========== An exception for when the input string can't be parsed into a valid snowflake ========================= */
class xBadSnowflakeError final : public std::invalid_argument {
	/* Private constructor so only cSnowflake can throw this exception */
	xBadSnowflakeError(std::string_view arg);
	friend class cSnowflake;
};
/* ========== Discord snowflake ===================================================================================== */
class cSnowflake {
	char          m_str[20]; // The snowflake as a string
	std::uint32_t m_len;     // The length of the string
	std::uint64_t m_int;     // The snowflake as a 64-bit integer

public:
	constexpr cSnowflake() noexcept : m_str("0"), m_len(1), m_int(0) {}
	constexpr cSnowflake(std::uint64_t arg) noexcept : m_str{}, m_int(arg) {
		auto [p, _] = std::to_chars(std::begin(m_str), std::end(m_str), arg);
		m_len = p - m_str;
	}
	template<typename Arg> requires std::is_convertible_v<Arg&&, const std::string_view&>
	constexpr cSnowflake(Arg&& arg) : m_str{}, m_int{} {
		const std::string_view& s = arg;
		const auto begin = s.data(), end = begin + s.size();
		if (auto [p, e] = std::from_chars(begin, end, m_int); e != std::errc{} || p != end)
			[[unlikely]]
			throw xBadSnowflakeError(s);
		m_len = s.copy(m_str, std::size(m_str));
	}

	constexpr auto operator<=>(const cSnowflake& o) const noexcept { return m_int <=> o.m_int; }
	constexpr bool operator== (const cSnowflake& o) const noexcept { return m_int ==  o.m_int; }

	/* Attributes */
	constexpr std::string_view ToString() const noexcept { return { m_str, m_len }; }
	constexpr std::uint64_t       ToInt() const noexcept { return m_int;            }

	/* Snowflake components - https://discord.com/developers/docs/reference#snowflakes */
	constexpr auto  GetInternalWorkerId() const noexcept { return static_cast<std::uint8_t >(0x01F & (m_int >> 17)); }
	constexpr auto GetInternalProcessId() const noexcept { return static_cast<std::uint8_t >(0x01F & (m_int >> 12)); }
	constexpr auto         GetIncrement() const noexcept { return static_cast<std::uint16_t>(0xFFF &  m_int       ); }
	/* Resource timestamp */
	constexpr auto GetTimestamp() const noexcept {
		using namespace std::chrono;
		using namespace std::chrono_literals;
		return sys_days(2015y/1/1) + milliseconds(m_int >> 22);
	}
};
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
#endif /* DISCORD_SNOWFLAKE_H */
