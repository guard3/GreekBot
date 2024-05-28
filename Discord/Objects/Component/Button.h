#ifndef DISCORD_BUTTON_H
#define DISCORD_BUTTON_H
#include "Common.h"
#include "Emoji.h"
#include <optional>

// TODO: Use only one button class
enum eButtonStyle {
	BUTTON_STYLE_PRIMARY = 1,
	BUTTON_STYLE_SECONDARY,
	BUTTON_STYLE_SUCCESS,
	BUTTON_STYLE_DANGER,
	// STYLE_LINK
};

KW_DECLARE(label, std::string)
KW_DECLARE(emoji, cEmoji)
KW_DECLARE(disabled, bool)

class cButton {
protected:
	int                   m_style;
	std::string           m_value;
	std::string           m_label;
	bool                  m_disabled;
	std::optional<cEmoji> m_emoji;

private:
	template<kw::key... Keys, typename T>
	cButton(eButtonStyle style, T&& value, kw::pack<Keys...> pack):
		m_style(style),
		m_value(std::forward<T>(value)),
		m_label(std::move(kw::get<"label">(pack))),
		m_disabled(kw::get<"disabled">(pack, false)) {
		if (auto p = kw::get_if<"emoji">(pack, kw::nullarg); p)
			m_emoji.emplace(std::move(*p));
	}

public:
	explicit cButton(const json::value&);
	explicit cButton(const json::object&);

	template<kw::key... Keys, typename Str = std::string> requires std::constructible_from<std::string, Str&&>
	cButton(eButtonStyle style, Str&& custom_id, kw::arg<Keys>&... kwargs) : cButton(style, std::forward<Str>(custom_id), kw::pack{ kwargs... }) {}

	eButtonStyle     GetStyle()    const noexcept { return (eButtonStyle)m_style;         }
	std::string_view GetCustomId() const noexcept { return m_value;                       }
	std::string_view GetLabel()    const noexcept { return m_label;                       }
	chEmoji          GetEmoji()    const noexcept { return m_emoji ? &*m_emoji : nullptr; }
	hEmoji           GetEmoji()          noexcept { return m_emoji ? &*m_emoji : nullptr; }
	bool             IsDisabled()  const noexcept { return m_disabled;                    }
};
cButton
tag_invoke(json::value_to_tag<cButton>, const json::value&);
void
tag_invoke(json::value_from_tag, json::value&, const cButton&);

class cLinkButton : public cButton {
private:
	using cButton::GetStyle;
	using cButton::GetCustomId;
public:
	explicit cLinkButton(const json::value&);
	explicit cLinkButton(const json::object&);

	template<kw::key... Keys, typename Str = std::string> requires std::constructible_from<std::string, Str&&>
	cLinkButton(Str&& url, kw::arg<Keys>&... kwargs) : cButton((eButtonStyle)5, std::forward<Str>(url), kwargs...) {}

	std::string_view GetUrl() const noexcept { return GetCustomId(); }
};
cLinkButton
tag_invoke(json::value_to_tag<cLinkButton>, const json::value&);
#endif /* DISCORD_BUTTON_H */
