#ifndef DISCORD_COLOR_H
#define DISCORD_COLOR_H
#include "ColorFwd.h"
/* ========== Color ================================================================================================= */
class cColor final {
	std::int32_t m_value;

public:
	constexpr cColor() noexcept : m_value{} {}
	constexpr cColor(std::int32_t v) noexcept : m_value(v) {}
	constexpr cColor(std::uint8_t r, std::uint8_t g, std::uint8_t b) noexcept : m_value((r << 16) | (g << 8) | b) {}

	constexpr std::uint8_t   GetRed(this cColor self) noexcept { return (self.m_value >> 16) & 0xFF; }
	constexpr std::uint8_t GetGreen(this cColor self) noexcept { return (self.m_value >>  8) & 0xFF; }
	constexpr std::uint8_t  GetBlue(this cColor self) noexcept { return (self.m_value      ) & 0xFF; }

	constexpr bool operator!(this cColor self) noexcept { return !self.m_value; }
	constexpr bool operator==(this cColor lhs, cColor rhs) noexcept { return lhs.m_value == rhs.m_value; }

	constexpr std::int32_t ToInt(this cColor self) noexcept { return self.m_value; }
	constexpr operator std::int32_t(this cColor self) noexcept { return self.ToInt(); }
};
cColor
tag_invoke(boost::json::value_to_tag<cColor>, const boost::json::value&);
void
tag_invoke(boost::json::value_from_tag, boost::json::value&, cColor);
#endif /* DISCORD_COLOR_H */