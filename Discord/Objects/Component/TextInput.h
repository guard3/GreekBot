#ifndef DISCORD_TEXTINPUT_H
#define DISCORD_TEXTINPUT_H
#include "Common.h"

enum eTextInputStyle : std::uint8_t {
	TEXT_INPUT_SHORT = 1,
	TEXT_INPUT_PARAGRAPH,
};
eTextInputStyle
tag_invoke(boost::json::value_to_tag<eTextInputStyle>, const boost::json::value&);
void
tag_invoke(boost::json::value_from_tag, boost::json::value&, eTextInputStyle);

class cTextInput final {
private:
	std::string     m_custom_id;
	std::string     m_label;
	std::string     m_value;
	eTextInputStyle m_style;
	bool            m_required;
	std::uint16_t   m_min_length;
	std::uint16_t   m_max_length;

public:
	explicit cTextInput(const boost::json::value&);
	explicit cTextInput(const boost::json::object&);

	template<typename Str1 = std::string, typename Str2 = std::string> requires requires {
		requires std::constructible_from<std::string, Str1&&>;
		requires std::constructible_from<std::string, Str2&&>;
	} cTextInput(eTextInputStyle style, Str1&& custom_id, Str2&& label, bool required = true):
		m_custom_id(std::forward<Str1>(custom_id)),
		m_label(std::forward<Str2>(label)),
		m_style(style),
		m_required(required),
		m_min_length(0),
		m_max_length(4000) {}
	/* Getters */
	std::string_view GetCustomId() const noexcept { return m_custom_id;  }
	std::string_view    GetLabel() const noexcept { return m_label;      }
	std::string_view    GetValue() const noexcept { return m_value;      }
	eTextInputStyle     GetStyle() const noexcept { return m_style;      }
	std::uint16_t   GetMinLength() const noexcept { return m_min_length; }
	std::uint16_t   GetMaxLength() const noexcept { return m_max_length; }

	bool IsRequired() const noexcept { return m_required; }
	/* Movers */
	std::string MoveCustomId() noexcept { return std::move(m_custom_id); }
	std::string    MoveLabel() noexcept { return std::move(m_label);     }
	std::string    MoveValue() noexcept { return std::move(m_value);     }
	/* Resetters */
	void  ResetRequired() noexcept { m_required = true;   }
	void ResetMinLength() noexcept { m_min_length = 0;    }
	void ResetMaxLength() noexcept { m_max_length = 4000; }
	/* Emplacers */
	// TODO: Implement
	/* Setters */
	// TODO: Implement
	cTextInput& SetMinLength(std::uint16_t arg) & noexcept { m_min_length = arg; return *this; }
	cTextInput& SetMaxLength(std::uint16_t arg) & noexcept { m_max_length = arg; return *this; }
	cTextInput&& SetMinLength(std::uint16_t arg) && noexcept { return std::move(SetMinLength(arg)); }
	cTextInput&& SetMaxLength(std::uint16_t arg) && noexcept { return std::move(SetMaxLength(arg)); }
};
cTextInput
tag_invoke(boost::json::value_to_tag<cTextInput>, const boost::json::value&);
void
tag_invoke(boost::json::value_from_tag, boost::json::value&, const cTextInput&);
#endif /* DISCORD_TEXTINPUT_H */