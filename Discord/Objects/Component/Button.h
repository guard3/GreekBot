#ifndef DISCORD_BUTTON_H
#define DISCORD_BUTTON_H
#include "Emoji.h"
#include <optional>

enum eButtonStyle : std::uint8_t {
	BUTTON_STYLE_PRIMARY = 1,
	BUTTON_STYLE_SECONDARY,
	BUTTON_STYLE_SUCCESS,
	BUTTON_STYLE_DANGER,
	BUTTON_STYLE_LINK
};
eButtonStyle
tag_invoke(json::value_to_tag<eButtonStyle>, const json::value&);
void
tag_invoke(json::value_from_tag, json::value&, eButtonStyle);

class cButton final {
	eButtonStyle          m_style;
	bool                  m_disabled;
	std::string           m_custom_id_or_url;
	std::string           m_label;
	std::optional<cEmoji> m_emoji;

public:
	explicit cButton(const json::value&);
	explicit cButton(const json::object&);

	template<typename Str1 = std::string, typename Str2 = std::string> requires requires {
		requires std::constructible_from<std::string, Str1&&>;
		requires std::constructible_from<std::string, Str2&&>;
	} cButton(eButtonStyle style, Str1&& custom_id_or_url, Str2&& label, bool disabled = false) :
		m_style(style),
		m_custom_id_or_url(std::forward<Str1>(custom_id_or_url)),
		m_label(std::forward<Str2>(label)),
		m_disabled(disabled) {}
	template<typename Str = std::string> requires std::constructible_from<std::string, Str&&>
	cButton(eButtonStyle style, Str&& custom_id_or_url, cEmoji emoji, bool disabled = false) :
		m_style(style),
		m_custom_id_or_url(std::forward<Str>(custom_id_or_url)),
		m_emoji(std::move(emoji)),
		m_disabled(disabled) {}

	eButtonStyle     GetStyle()    const noexcept { return m_style;         }
	std::string_view GetCustomId() const noexcept { return m_style == BUTTON_STYLE_LINK ? std::string_view{} : m_custom_id_or_url; }
	std::string_view GetUrl()      const noexcept { return m_style == BUTTON_STYLE_LINK ? m_custom_id_or_url : std::string_view{}; }
	std::string_view GetLabel()    const noexcept { return m_label;                       }
	chEmoji          GetEmoji()    const noexcept { return m_emoji ? &*m_emoji : nullptr; }
	hEmoji           GetEmoji()          noexcept { return m_emoji ? &*m_emoji : nullptr; }
	bool             IsDisabled()  const noexcept { return m_disabled;                    }

	std::string MoveCustomId() noexcept { return m_style == BUTTON_STYLE_LINK ? std::string() : std::move(m_custom_id_or_url); }
	std::string MoveUrl() noexcept { return m_style == BUTTON_STYLE_LINK ? std::move(m_custom_id_or_url) : std::string(); }
	std::string MoveLabel() noexcept { return std::move(m_label); }

	cButton& SetStyle(eButtonStyle style) & noexcept {
		if ((m_style != BUTTON_STYLE_LINK) != (style != BUTTON_STYLE_LINK))
			m_custom_id_or_url.clear();
		m_style = style;
		return *this;
	}
	template<typename Arg = std::string> requires std::constructible_from<std::string, Arg&&>
	cButton& SetCustomId(Arg&& arg) & {
		if (m_style != BUTTON_STYLE_LINK)
			m_custom_id_or_url = std::forward<Arg>(arg);
		return *this;
	}
	template<typename Arg = std::string> requires std::constructible_from<std::string, Arg&&>
	cButton& SetUrl(Arg&& arg) & {
		if (m_style == BUTTON_STYLE_LINK)
			m_custom_id_or_url = std::forward<Arg>(arg);
		return *this;
	}
	template<typename Arg = std::string> requires std::constructible_from<std::string, Arg&&>
	cButton& SetLabel(Arg&& arg) & {
		m_label = std::forward<Arg>(arg);
		return *this;
	}
	template<typename Arg = cEmoji> requires std::constructible_from<cEmoji, Arg&&>
	cButton& SetEmoji(Arg&& arg) & {
		m_emoji.emplace(std::forward<Arg>(arg));
		return *this;
	}

	cButton&& SetStyle(eButtonStyle style) && noexcept { return std::move(SetStyle(style));	}
	template<typename Arg = std::string> requires std::constructible_from<std::string, Arg&&>
	cButton&& SetCustomId(Arg&& arg) && { return std::move(SetCustomId(std::forward<Arg>(arg))); }
	template<typename Arg = std::string> requires std::constructible_from<std::string, Arg&&>
	cButton&& SetUrl(Arg&& arg) && { return std::move(SetUrl(std::forward<Arg>(arg))); }
	template<typename Arg = std::string> requires std::constructible_from<std::string, Arg&&>
	cButton&& SetLabel(Arg&& arg) && { return std::move(SetLabel(std::forward<Arg>(arg))); }
	template<typename Arg = cEmoji> requires std::constructible_from<cEmoji, Arg&&>
	cButton&& SetEmoji(Arg&& arg) && { return std::move(SetEmoji(std::forward<Arg>(arg))); }
};
cButton
tag_invoke(json::value_to_tag<cButton>, const json::value&);
void
tag_invoke(json::value_from_tag, json::value&, const cButton&);
#endif /* DISCORD_BUTTON_H */
