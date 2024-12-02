#ifndef DISCORD_EMBEDFIELD_H
#define DISCORD_EMBEDFIELD_H
#include "Base.h"

class cEmbedField final {
	std::string m_name, m_value;
	bool        m_inline;

public:
	explicit cEmbedField(const boost::json::value&);
	explicit cEmbedField(const boost::json::object&);

	template<typename Str1 = std::string, typename Str2 = std::string> requires requires {
		requires std::constructible_from<std::string, Str1&&>;
		requires std::constructible_from<std::string, Str2&&>;
	} cEmbedField(Str1&& name, Str2&& value, bool inline_ = false):
		m_name(std::forward<Str1>(name)),
		m_value(std::forward<Str2>(value)),
		m_inline(inline_) {}
	/* Getters */
	std::string_view  GetName() const noexcept { return m_name;   }
	std::string_view GetValue() const noexcept { return m_value;  }
	bool             IsInline() const noexcept { return m_inline; }
	/* Movers */
	std::string  MoveName() noexcept { return std::move(m_name ); }
	std::string MoveValue() noexcept { return std::move(m_value); }
	/* Resetters */
	void ResetInline() noexcept { m_inline = false;	}
	/* Emplacers */
	std::string& EmplaceName() noexcept {
		m_name.clear();
		return m_name;
	}
	std::string& EmplaceValue() noexcept {
		m_value.clear();
		return m_value;
	}
	template<typename Arg = std::string, typename... Args> requires std::constructible_from<std::string, Arg&&, Args&&...>
	std::string& EmplaceName(Arg&& arg, Args&&... args) {
		if constexpr (std::assignable_from<std::string&, Arg&&> && sizeof...(args) == 0)
			return m_name = std::forward<Arg>(arg);
		else
			return m_name = std::string(std::forward<Arg>(arg), std::forward<Args>(args)...);
	}
	template<typename Arg = std::string, typename... Args> requires std::constructible_from<std::string, Arg&&, Args&&...>
	std::string& EmplaceValue(Arg&& arg, Args&&... args) {
		if constexpr (std::assignable_from<std::string&, Arg&&> && sizeof...(args) == 0)
			return m_name = std::forward<Arg>(arg);
		else
			return m_name = std::string(std::forward<Arg>(arg), std::forward<Args>(args)...);
	}
	/* Setters */
	template<typename Self, typename Arg = std::string> requires std::constructible_from<std::string, Arg&&>
	Self&& SetName(this Self&& self, Arg&& arg) {
		self.EmplaceName(std::forward<Arg>(arg));
		return std::forward<Self>(self);
	}
	template<typename Self, typename Arg = std::string> requires std::constructible_from<std::string, Arg&&>
	Self&& SetValue(this Self&& self, Arg&& arg) {
		self.EmplaceValue(std::forward<Arg>(arg));
		return std::forward<Self>(self);
	}
	template<typename Self>
	Self&& SetInline(this Self&& self, bool inline_) noexcept {
		self.m_inline = inline_;
		return std::forward<Self>(self);
	}
};

cEmbedField
tag_invoke(boost::json::value_to_tag<cEmbedField>, const boost::json::value&);
void
tag_invoke(boost::json::value_from_tag, boost::json::value&, const cEmbedField&);
#endif /* DISCORD_EMBEDFIELD_H */