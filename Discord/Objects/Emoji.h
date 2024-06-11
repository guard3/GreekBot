#ifndef DISCORD_EMOJI_H
#define DISCORD_EMOJI_H
#include "EmojiFwd.h"

class cEmoji final {
	cSnowflake  m_id;
	std::string m_name;
	bool        m_animated;

public:
	explicit cEmoji(const boost::json::value&);
	explicit cEmoji(const boost::json::object&);

	template<typename Str = std::string> requires std::constructible_from<std::string, Str&&>
	explicit cEmoji(Str&& unicode_emoji) : m_name(std::forward<Str>(unicode_emoji)), m_animated(false) {}

	template<typename Str = std::string> requires std::constructible_from<std::string, Str&&>
	cEmoji(Str&& name, const cSnowflake& id, bool animated = false):
		m_name(std::forward<Str>(name)),
		m_id(id),
		m_animated(animated) {}

	std::string_view GetName() const noexcept { return m_name; }
	chSnowflake        GetId() const noexcept { return m_id.ToInt() ? &m_id : nullptr; }
	bool          IsAnimated() const noexcept { return m_animated; }

	hSnowflake GetId() noexcept { return m_id.ToInt() ? &m_id : nullptr; }

	std::string ToString() const;
};

cEmoji
tag_invoke(boost::json::value_to_tag<cEmoji>, const boost::json::value& v);
void
tag_invoke(boost::json::value_from_tag, boost::json::value&, const cEmoji&);
#endif /* DISCORD_EMOJI_H */