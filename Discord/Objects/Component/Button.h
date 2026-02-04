#ifndef DISCORD_BUTTON_H
#define DISCORD_BUTTON_H
#include "ComponentBase.h"
#include "ComponentFwd.h"
#include "Emoji.h"
#include <optional>

enum class eButtonStyle : std::uint8_t {
	Primary = 1,
	Secondary,
	Success,
	Danger,
	Link
};

class cButton : public cComponentBase {
	eButtonStyle          m_style; // Must be defined first so that it gets initialized before m_custom_id_or_url
	bool                  m_disabled;
	std::string           m_custom_id_or_url;
	std::string           m_label;
	std::optional<cEmoji> m_emoji;

	template<typename Self, typename Arg>
	Self&& set(this Self&& self, std::string& str, Arg&& arg) {
		if constexpr (std::is_assignable_v<std::string&, Arg&&>) {
			str = std::forward<Arg>(arg);
		} else {
			str = std::string(std::forward<Arg>(arg));
		}
		return std::forward<Self>(self);
	}

public:
	explicit cButton(const boost::json::value&);
	explicit cButton(const boost::json::object&);

	template<iExplicitlyConvertibleTo<std::string> Str1 = std::string, iExplicitlyConvertibleTo<std::string> Str2 = std::string>
	cButton(eButtonStyle style, Str1&& custom_id_or_url, Str2&& label, bool disabled = false) :
		m_style(style),
		m_disabled(disabled),
		m_custom_id_or_url(std::forward<Str1>(custom_id_or_url)),
		m_label(std::forward<Str2>(label)) {}
	template<iExplicitlyConvertibleTo<std::string> Str = std::string>
	cButton(eButtonStyle style, Str&& custom_id_or_url, cEmoji emoji, bool disabled = false) :
		m_style(style),
		m_disabled(disabled),
		m_custom_id_or_url(std::forward<Str>(custom_id_or_url)),
		m_emoji(std::move(emoji)) {}

	eButtonStyle GetStyle() const noexcept { return m_style; }
	std::string_view GetCustomId() const noexcept { return m_style == eButtonStyle::Link ? std::string_view{} : m_custom_id_or_url; }
	std::string_view GetUrl() const noexcept { return m_style == eButtonStyle::Link ? m_custom_id_or_url : std::string_view{}; }
	std::string_view GetLabel() const noexcept { return m_label; }
	auto GetEmoji(this auto&& self) noexcept { return cPtr(self.m_emoji ? self.m_emoji.operator->() : nullptr);}
	bool IsDisabled() const noexcept { return m_disabled; }

	std::string MoveCustomId() noexcept { return m_style == eButtonStyle::Link ? std::string() : std::move(m_custom_id_or_url); }
	std::string MoveUrl() noexcept { return m_style == eButtonStyle::Link ? std::move(m_custom_id_or_url) : std::string(); }
	std::string MoveLabel() noexcept { return std::move(m_label); }

	template<typename Self>
	Self&& SetStyle(this Self&& self, eButtonStyle style) noexcept {
		if ((self.m_style != eButtonStyle::Link) != (style != eButtonStyle::Link))
			self.m_custom_id_or_url.clear();
		self.m_style = style;
		return std::forward<Self>(self);
	}
	template<typename Self, iExplicitlyConvertibleTo<std::string> Arg = std::string>
	Self&& SetCustomId(this Self&& self, Arg&& arg) {
		return self.m_style == eButtonStyle::Link
		                     ? std::forward<Self>(self)
		                     : std::forward<Self>(self).set(self.m_custom_id_or_url, std::forward<Arg>(arg));
	}
	template<typename Self, iExplicitlyConvertibleTo<std::string> Arg = std::string>
	Self&& SetUrl(this Self&& self, Arg&& arg) {
		return self.m_style == eButtonStyle::Link
		                     ? std::forward<Self>(self).set(self.m_custom_id_or_url, std::forward<Arg>(arg))
		                     : std::forward<Self>(self);
	}
	template<typename Self, typename Arg = std::string> requires std::constructible_from<std::string, Arg&&>
	Self&& SetLabel(this Self&& self, Arg&& arg) {
		return std::forward<Self>(self).set(self.m_label, std::forward<Arg>(arg));
	}
	template<typename Self, typename Arg = cEmoji> requires std::constructible_from<cEmoji, Arg&&>
	Self&& SetEmoji(this Self&& self, Arg&& arg) {
		self.m_emoji.emplace(std::forward<Arg>(arg));
		return std::forward<Self>(self);
	}
};

/** @name JSON value to object conversion
 */
/// @{
eButtonStyle
tag_invoke(boost::json::value_to_tag<eButtonStyle>, const boost::json::value&);

cButton
tag_invoke(boost::json::value_to_tag<cButton>, const boost::json::value&);
/// @}

/** @name JSON value from object conversion
 */
/// @{
void
tag_invoke(boost::json::value_from_tag, boost::json::value&, eButtonStyle);

void
tag_invoke(boost::json::value_from_tag, boost::json::value&, const cButton&);
/// @}
#endif /* DISCORD_BUTTON_H */
