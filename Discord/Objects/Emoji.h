#ifndef DISCORD_EMOJI_H
#define DISCORD_EMOJI_H
#include "Base.h"
#include "EmojiFwd.h"

class cEmoji final {
	cSnowflake  m_id;
	std::string m_name;
	bool        m_animated;

public:
	explicit cEmoji(const boost::json::value&);
	explicit cEmoji(const boost::json::object&);

	template<typename Str = std::string> requires std::constructible_from<std::string, Str&&>
	explicit constexpr cEmoji(Str&& unicode_emoji) : m_name(std::forward<Str>(unicode_emoji)), m_animated(false) {}

	template<typename Str = std::string> requires std::constructible_from<std::string, Str&&>
	constexpr cEmoji(Str&& name, const cSnowflake& id, bool animated = false):
		m_id(id),
		m_name(std::forward<Str>(name)),
		m_animated(animated) {}

	bool operator==(const cEmoji&) const = default;

	std::string_view GetName() const noexcept { return m_name; }
	auto GetId(this auto&& self) noexcept { return cPtr(self.m_id.ToInt() ? &self.m_id : nullptr); }
	bool IsAnimated() const noexcept { return m_animated; }

	std::string ToString() const;
};

cEmoji
tag_invoke(boost::json::value_to_tag<cEmoji>, const boost::json::value& v);
void
tag_invoke(boost::json::value_from_tag, boost::json::value&, const cEmoji&);

template<>
struct std::formatter<cEmoji> {
	bool m_bPct{};

	constexpr auto parse(std::format_parse_context& ctx) {
		const auto begin = ctx.begin(), end = std::ranges::find(ctx, '}');
		if (begin != end) {
			if (std::string_view{ begin, end } != "pct")
				throw std::format_error("invalid cEmoji format specifier (use pct)");

			m_bPct = true;
		}
		return end;
	}

	std::format_context::iterator format(const cEmoji& emoji, std::format_context& ctx) const;
};
#endif /* DISCORD_EMOJI_H */
