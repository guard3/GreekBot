#ifndef DISCORD_SELECTMENU_H
#define DISCORD_SELECTMENU_H
#include "ComponentBase.h"
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
	explicit cSelectOption(const boost::json::value&);
	explicit cSelectOption(const boost::json::object&);

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
	auto GetEmoji(this auto&& self) noexcept { return cPtr(self.m_emoji ? self.m_emoji.operator->() : nullptr); }
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
	template<typename Self, iExplicitlyConvertibleTo<std::string> Arg = std::string>
	Self&& SetLabel(this Self&& self, Arg&& arg) {
		self.EmplaceLabel(std::forward<Arg>(arg));
		return std::forward<Self>(self);
	}

	template<typename Self, iExplicitlyConvertibleTo<std::string> Arg = std::string>
	Self&& SetValue(this Self&& self, Arg&& arg) {
		self.EmplaceValue(std::forward<Arg>(arg));
		return std::forward<Self>(self);
	}

	template<typename Self, iExplicitlyConvertibleTo<std::string> Arg = std::string>
	Self&& SetDescription(this Self&& self, Arg&& arg) {
		self.EmplaceDescription(std::forward<Arg>(arg));
		return std::forward<Self>(self);
	}

	template<typename Self, iExplicitlyConvertibleTo<std::string> Arg = std::string>
	Self&& SetEmoji(this Self&& self, Arg&& arg) {
		self.EmplaceEmoji(std::forward<Arg>(arg));
		return std::forward<Self>(self);
	}
};

class cSelectMenu : public cComponentBase {
	std::string m_custom_id;
	std::string m_placeholder;
	std::vector<cSelectOption> m_options;

public:
	explicit cSelectMenu(const boost::json::value&);
	explicit cSelectMenu(const boost::json::object&);

	template<typename Str1 = std::string, typename Str2 = std::string, typename Vec = std::vector<cSelectOption>> requires requires {
		requires std::constructible_from<std::string, Str1&&>;
		requires std::constructible_from<std::string, Str2&&>;
		requires std::constructible_from<std::vector<cSelectOption>, Vec&&>;
	} cSelectMenu(Str1&& custom_id, Vec&& options, Str2&& placeholder = {}):
		m_custom_id(std::forward<Str1>(custom_id)),
		m_placeholder(std::forward<Str2>(placeholder)),
		m_options(std::forward<Vec>(options)) {}
	/* Getters */
	std::string_view    GetCustomId() const noexcept { return m_custom_id;   }
	std::string_view GetPlaceholder() const noexcept { return m_placeholder; }
	auto GetOptions(this auto&& self) noexcept { return std::span(self.m_options); }
	/* Movers */
	auto    MoveCustomId() noexcept { return std::move(m_custom_id);   }
	auto MovePlaceholder() noexcept { return std::move(m_placeholder); }
	auto     MoveOptions() noexcept { return std::move(m_options);     }
};

/** @name Value to JSON conversions
 */
/// @{
cSelectOption
tag_invoke(boost::json::value_to_tag<cSelectOption>, const boost::json::value&);

cSelectMenu
tag_invoke(boost::json::value_to_tag<cSelectMenu>, const boost::json::value&);
/// @}

/** @name Value from JSON conversions
 */
/// @{
void
tag_invoke(boost::json::value_from_tag, boost::json::value&, const cSelectOption&);

void
tag_invoke(boost::json::value_from_tag, boost::json::value&, const cSelectMenu&);
/// @}
#endif /* DISCORD_SELECTMENU_H */