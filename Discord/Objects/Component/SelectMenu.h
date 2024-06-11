#ifndef DISCORD_SELECTMENU_H
#define DISCORD_SELECTMENU_H
#include "Emoji.h"
#include <optional>
#include <vector>
#include <span>

class cSelectOption final {
	std::string           m_label;
	std::string           m_value;
	std::string           m_description;
	std::optional<cEmoji> m_emoji;

public:
	explicit cSelectOption(const json::value&);
	explicit cSelectOption(const json::object&);

	template<typename Str1 = std::string, typename Str2 = std::string, typename Str3 = std::string> requires requires {
		requires std::constructible_from<std::string, Str1&&>;
		requires std::constructible_from<std::string, Str2&&>;
		requires std::constructible_from<std::string, Str3&&>;
	} cSelectOption(Str1&& label, Str2&& value, Str3&& description = {}):
		m_label(std::forward<Str1>(label)),
		m_value(std::forward<Str2>(value)),
		m_description(std::forward<Str3>(description)) {}
	/* Getters */
	std::string_view       GetLabel() const noexcept { return m_label;       }
	std::string_view       GetValue() const noexcept { return m_value;       }
	std::string_view GetDescription() const noexcept { return m_description; }
	chEmoji                GetEmoji() const noexcept { return m_emoji ? m_emoji.operator->() : nullptr; }
	 hEmoji                GetEmoji()       noexcept { return m_emoji ? m_emoji.operator->() : nullptr; }
	/* Movers */
	std::string       MoveLabel() noexcept { return std::move(m_label);       }
	std::string       MoveValue() noexcept { return std::move(m_value);       }
	std::string MoveDescription() noexcept { return std::move(m_description); }
	/* Resetters */
	void ResetDescription() noexcept { m_description.clear(); }
	void       ResetEmoji() noexcept { m_emoji.reset();       }
	/* Emplacers */
	std::string& EmplaceLabel() noexcept {
		m_label.clear();
		return m_label;
	}
	std::string& EmplaceValue() noexcept {
		m_value.clear();
		return m_value;
	}
	std::string& EmplaceDescription() noexcept {
		m_description.clear();
		return m_description;
	}
	template<typename Arg = std::string, typename... Args> requires std::constructible_from<std::string, Arg&&, Args&&...>
	std::string& EmplaceLabel(Arg&& arg, Args&&... args) {
		if constexpr (std::assignable_from<std::string&, Arg&&> && sizeof...(args) == 0)
			return m_label = std::forward<Arg>(arg);
		else
			return m_label = std::string(std::forward<Arg>(arg), std::forward<Args>(args)...);
	}
	template<typename Arg = std::string, typename... Args> requires std::constructible_from<std::string, Arg&&, Args&&...>
	std::string& EmplaceValue(Arg&& arg, Args&&... args) {
		if constexpr (std::assignable_from<std::string&, Arg&&> && sizeof...(args) == 0)
			return m_value = std::forward<Arg>(arg);
		else
			return m_value = std::string(std::forward<Arg>(arg), std::forward<Args>(args)...);
	}
	template<typename Arg = std::string, typename... Args> requires std::constructible_from<std::string, Arg&&, Args&&...>
	std::string& EmplaceDescription(Arg&& arg, Args&&... args) {
		if constexpr (std::assignable_from<std::string&, Arg&&> && sizeof...(args) == 0)
			return m_description = std::forward<Arg>(arg);
		else
			return m_description = std::string(std::forward<Arg>(arg), std::forward<Args>(args)...);
	}
	template<typename Arg = cEmoji, typename... Args> requires std::constructible_from<cEmoji, Arg&&, Args&&...>
	cEmoji& EmplaceEmoji(Arg&& arg, Args&&... args) {
		if constexpr (std::assignable_from<cEmoji&, Arg&&> && sizeof...(args) == 0)
			return m_emoji = std::forward<Arg>(arg);
		else
			return m_emoji.emplace(std::forward<Arg>(arg), std::forward<Arg>(args)...);
	}
	/* Setters */
	template<typename Arg = std::string> requires std::constructible_from<std::string, Arg&&>
	cSelectOption& SetLabel(Arg&& arg) & { EmplaceLabel(std::forward<Arg>(arg)); return *this; }
	template<typename Arg = std::string> requires std::constructible_from<std::string, Arg&&>
	cSelectOption& SetValue(Arg&& arg) & { EmplaceValue(std::forward<Arg>(arg)); return *this; }
	template<typename Arg = std::string> requires std::constructible_from<std::string, Arg&&>
	cSelectOption& SetDescription(Arg&& arg) & { EmplaceDescription(std::forward<Arg>(arg)); return *this; }
	template<typename Arg = cEmoji> requires std::constructible_from<cEmoji, Arg&&>
	cSelectOption& SetEmoji(Arg&& arg) & { EmplaceEmoji(std::forward<Arg>(arg)); return *this; }
	template<typename Arg = std::string> requires std::constructible_from<std::string, Arg&&>
	cSelectOption&& SetLabel(Arg&& arg) && { return std::move(SetLabel(std::forward<Arg>(arg))); }
	template<typename Arg = std::string> requires std::constructible_from<std::string, Arg&&>
	cSelectOption&& SetValue(Arg&& arg) && { return std::move(SetValue(std::forward<Arg>(arg))); }
	template<typename Arg = std::string> requires std::constructible_from<std::string, Arg&&>
	cSelectOption&& SetDescription(Arg&& arg) && { return std::move(SetDescription(std::forward<Arg>(arg))); }
	template<typename Arg = cEmoji> requires std::constructible_from<cEmoji, Arg&&>
	cSelectOption&& SetEmoji(Arg&& arg) && { return std::move(SetEmoji(std::forward<Arg>(arg))); }
};
cSelectOption
tag_invoke(json::value_to_tag<cSelectOption>, const json::value&);
void
tag_invoke(json::value_from_tag, json::value&, const cSelectOption&);

class cSelectMenu final {
	std::string m_custom_id;
	std::string m_placeholder;
	std::vector<cSelectOption> m_options;

public:
	explicit cSelectMenu(const json::value&);
	explicit cSelectMenu(const json::object&);

	template<typename Str1 = std::string, typename Str2 = std::string, typename Vec = std::vector<cSelectOption>> requires requires {
		requires std::constructible_from<std::string, Str1&&>;
		requires std::constructible_from<std::string, Str2&&>;
		requires std::constructible_from<std::vector<cSelectOption>, Vec&&>;
	} cSelectMenu(Str1&& custom_id, Vec&& options, Str2&& placeholder = {}):
		m_custom_id(std::forward<Str1>(custom_id)),
		m_placeholder(std::forward<Str2>(placeholder)),
		m_options(std::forward<Vec>(options)) {}
	/* Getters */
	std::string_view              GetCustomId() const noexcept { return m_custom_id;   }
	std::string_view           GetPlaceholder() const noexcept { return m_placeholder; }
	std::span<const cSelectOption> GetOptions() const noexcept { return m_options;     }
	std::span<      cSelectOption> GetOptions()       noexcept { return m_options;     }
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
