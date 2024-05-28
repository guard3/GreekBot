#ifndef DISCORD_SELECTMENU_H
#define DISCORD_SELECTMENU_H
#include "Common.h"
#include "Emoji.h"
#include <optional>
#include <vector>
#include <span>

class cSelectOption final {
private:
	std::string           m_label;
	std::string           m_value;
	std::string           m_description;
	std::optional<cEmoji> m_emoji;

	template<kw::key... Keys, typename S1, typename S2>
	cSelectOption(S1&& l, S2&& v, kw::pack<Keys...> pack):
		m_label(std::forward<S1>(l)),
		m_value(std::forward<S2>(v)),
		m_description(std::move(kw::get<"description">(pack))) {
		// TODO: Figure out how *not* to use kwargs here...
		//if (auto p = kw::get_if<"emoji">(pack, kw::nullarg))
		//	m_emoji.emplace(std::move(*p));
	}

public:
	explicit cSelectOption(const json::value&);
	explicit cSelectOption(const json::object&);

	template<kw::key... Keys, typename Str1 = std::string, typename Str2 = std::string>
	requires std::constructible_from<std::string, Str1&&> && std::constructible_from<std::string, Str2&&>
	cSelectOption(Str1&& label, Str2&& value, kw::arg<Keys>&... kwargs) : cSelectOption(std::forward<Str1>(label), std::forward<Str2>(value), kw::pack{ kwargs... }) {}
	/* Getters */
	std::string_view       GetLabel() const noexcept { return m_label;       }
	std::string_view       GetValue() const noexcept { return m_value;       }
	std::string_view GetDescription() const noexcept { return m_description; }
	chEmoji                GetEmoji() const noexcept { return m_emoji ? m_emoji.operator->() : nullptr; }
	/* Movers */
	std::string       MoveLabel() noexcept { return std::move(m_label);       }
	std::string       MoveValue() noexcept { return std::move(m_value);       }
	std::string MoveDescription() noexcept { return std::move(m_description); }
};
cSelectOption
tag_invoke(json::value_to_tag<cSelectOption>, const json::value&);
void
tag_invoke(json::value_from_tag, json::value&, const cSelectOption&);

KW_DECLARE(placeholder, std::string)

class cSelectMenu final {
private:
	std::string m_custom_id;
	std::string m_placeholder;
	std::vector<cSelectOption> m_options;

	template<kw::key... Keys, typename S = std::string, typename V>
	cSelectMenu(S&& s, V&& v, kw::pack<Keys...> pack):
		m_custom_id(std::forward<S>(s)),
		m_options(std::forward<V>(v)),
		m_placeholder(std::move(kw::get<"placeholder">(pack))) {}

public:
	explicit cSelectMenu(const json::value&);
	explicit cSelectMenu(const json::object&);

	template<kw::key... Keys, typename Str = std::string> requires std::constructible_from<std::string, Str&&>
	cSelectMenu(Str&& custom_id, std::vector<cSelectOption> options, kw::arg<Keys>&... kwargs): cSelectMenu(std::forward<Str>(custom_id), std::move(options), kw::pack{ kwargs... }) {}
	/* Getters */
	std::string_view              GetCustomId() const noexcept { return m_custom_id;   }
	std::string_view           GetPlaceholder() const noexcept { return m_placeholder; }
	std::span<const cSelectOption> GetOptions() const noexcept { return m_options;     }
	/* Movers */
	std::string               MoveCustomId() noexcept { return std::move(m_custom_id);   }
	std::string            MovePlaceholder() noexcept { return std::move(m_placeholder); }
	std::vector<cSelectOption> MoveOptions() noexcept { return std::move(m_options);     }
};
cSelectMenu
tag_invoke(json::value_to_tag<cSelectMenu>, const json::value&);
void
tag_invoke(json::value_from_tag, json::value&, const cSelectMenu&);
#endif /* DISCORD_SELECTMENU_H */
