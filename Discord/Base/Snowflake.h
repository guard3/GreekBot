#ifndef DISCORD_SNOWFLAKE_H
#define DISCORD_SNOWFLAKE_H
#include "SnowflakeFwd.h"
#include <algorithm>
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
/* ========== Make cSnowflake hashable ============================================================================== */
template<>
struct std::hash<cSnowflake> : std::hash<std::uint64_t> {
	std::size_t operator()(const cSnowflake& snowflake) const noexcept {
		return static_cast<const std::hash<std::uint64_t>&>(*this)(snowflake.ToInt());
	}
};
/* ========== Make cSnowflake formattable =========================================================================== */
template<>
struct std::formatter<cSnowflake> {
	std::string_view m_prefix;

	constexpr auto parse(std::format_parse_context& ctx) {
		auto begin = ctx.begin(), end = std::find(begin, ctx.end(), '}');
		if (begin != end) {
			if (std::string_view spec{ begin, end }; spec == "u" || spec == "user")
				m_prefix = "@";
			else if (spec == "r" || spec == "role")
				m_prefix = "@&";
			else if (spec == "c" || spec == "channel")
				m_prefix = "#";
			else
				throw std::format_error("invalid cSnowflake format specifier (use u/user, r/role, c/channel)");
		}
		return end;
	}

	auto format(const cSnowflake& sf, std::format_context& ctx) const {
		auto out = ctx.out();
		auto str = sf.ToString();

		// If no mention prefix is specified, print the snowflake as a plain string
		if (m_prefix.empty())
			return std::copy(str.begin(), str.end(), out);

		*out++ = '<';
		out = std::copy(m_prefix.begin(), m_prefix.end(), out);
		out = std::copy(str.begin(), str.end(), out);
		*out++ = '>';

		return out;
	}
};
#endif /* DISCORD_SNOWFLAKE_H */
