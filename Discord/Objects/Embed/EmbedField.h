#ifndef GREEKBOT_EMBEDFIELD_H
#define GREEKBOT_EMBEDFIELD_H
#include "Common.h"

class cEmbedField final {
private:
	std::string m_name, m_value;
	bool        m_inline;

public:
	explicit cEmbedField(const json::value&);
	template<typename Str1 = std::string, typename Str2 = std::string>
	requires std::constructible_from<std::string, Str1&&> && std::constructible_from<std::string, Str2&&>
	cEmbedField(Str1&& name, Str2&& value, bool inline_ = false):
			m_name(std::forward<Str1>(name)),
			m_value(std::forward<Str2>(value)),
			m_inline(inline_) {}
	/* Getters */
	std::string_view GetName()  const noexcept { return m_name;   }
	std::string_view GetValue() const noexcept { return m_value;  }
	bool             IsInline() const noexcept { return m_inline; }
	/* Movers */
	std::string MoveName()  noexcept { return std::move(m_name ); }
	std::string MoveValue() noexcept { return std::move(m_value); }
	/* Setters */
	template<typename Str = std::string> requires std::assignable_from<std::string&, Str&&>
	cEmbedField& SetName(Str&& arg) {
		m_name = std::forward<Str>(arg);
		return *this;
	}
	template<typename Str = std::string> requires std::assignable_from<std::string&, Str&&>
	cEmbedField& SetValue(Str&& arg) {
		m_value = std::forward<Str>(arg);
		return *this;
	}
	cEmbedField& SetInline(bool inline_) {
		m_inline = inline_;
		return *this;
	}
};
typedef   hHandle<cEmbedField>   hEmbedField;
typedef  chHandle<cEmbedField>  chEmbedField;
typedef  uhHandle<cEmbedField>  uhEmbedField;
typedef uchHandle<cEmbedField> uchEmbedField;
#endif /* GREEKBOT_EMBEDFIELD_H */
