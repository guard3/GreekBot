#ifndef DISCORD_COLOR_H
#define DISCORD_COLOR_H
#include "ColorFwd.h"
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
cColor
tag_invoke(boost::json::value_to_tag<cColor>, const boost::json::value&);
void
tag_invoke(boost::json::value_from_tag, boost::json::value&, cColor);
#endif /* DISCORD_COLOR_H */