#ifndef GREEKBOT_EMOJI_H
#define GREEKBOT_EMOJI_H
#include "Common.h"
#include <optional>
//TODO: separate guild emoji and standard emoji types
class cEmoji final {
private:
	std::string               m_name;
	std::optional<cSnowflake> m_id;
	bool                      m_animated;

public:
	explicit cEmoji(const json::value&);
	template<typename Str = std::string> requires std::constructible_from<std::string, Str&&>
	cEmoji(Str&& name, const cSnowflake& id, bool animated = false):
		m_name(std::forward<Str>(name)),
		m_id(id),
		m_animated(animated) {}

	std::string_view GetName() const noexcept { return m_name; }
	chSnowflake        GetId() const noexcept { return m_id.has_value() ? &*m_id : nullptr; }
	bool          IsAnimated() const noexcept { return m_animated; }

	std::string ToString() const;
};
typedef   hHandle<cEmoji>   hEmoji;
typedef  chHandle<cEmoji>  chEmoji;
typedef  uhHandle<cEmoji>  uhEmoji;
typedef uchHandle<cEmoji> uchEmoji;
typedef  shHandle<cEmoji>  shEmoji;
typedef schHandle<cEmoji> schEmoji;

void tag_invoke(const json::value_from_tag&, json::value&, const cEmoji&);
#endif //GREEKBOT_EMOJI_H
