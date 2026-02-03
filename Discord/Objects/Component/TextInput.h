#ifndef DISCORD_TEXTINPUT_H
#define DISCORD_TEXTINPUT_H
#include "ComponentBase.h"

enum eTextInputStyle : std::uint8_t {
	TEXT_INPUT_SHORT = 1,
	TEXT_INPUT_PARAGRAPH,
};

/**
 * Partial Text Input is an interaction response object. It is received from modal interactions
 */
struct cPartialTextInput : cComponentBase {
	explicit cPartialTextInput(const boost::json::value&);
	explicit cPartialTextInput(const boost::json::object&);

	/* Getters */
	std::string_view GetValue()    const noexcept { return m_value;     }
	std::string_view GetCustomId() const noexcept { return m_custom_id; }

	/* Movers */
	std::string MoveValue()    noexcept { return std::move(m_value);     }
	std::string MoveCustomId() noexcept { return std::move(m_custom_id); }

protected:
	std::string m_custom_id;
	std::string m_value;

	template<typename Str>
	explicit cPartialTextInput(Str&& custom_id) : m_custom_id(std::forward<Str>(custom_id)) {}
};

/**
 * Text Input is an interactive component that allows users to enter free-form text responses in modals.
 * It supports both short, single-line inputs and longer, multi-line paragraph inputs.
 */
class cTextInput : public cPartialTextInput {
	eTextInputStyle m_style;
	bool            m_required;
	std::uint16_t   m_min_length = 0;
	std::uint16_t   m_max_length = 4000;

public:
	template<iExplicitlyConvertibleTo<std::string> Str = std::string>
	cTextInput(eTextInputStyle style, Str&& custom_id, bool required = true):
		cPartialTextInput(std::forward<Str>(custom_id)),
		m_style(style),
		m_required(required) {}

	/* Getters */
	eTextInputStyle     GetStyle() const noexcept { return m_style;      }
	std::uint16_t   GetMinLength() const noexcept { return m_min_length; }
	std::uint16_t   GetMaxLength() const noexcept { return m_max_length; }

	bool IsRequired() const noexcept { return m_required; }
	/* Resetters */
	void  ResetRequired() noexcept { m_required = true;   }
	void ResetMinLength() noexcept { m_min_length = 0;    }
	void ResetMaxLength() noexcept { m_max_length = 4000; }
	/* Emplacers */
	// TODO: Implement
	/* Setters */
	// TODO: Implement
	auto&& SetMinLength(this auto&& self, std::uint16_t arg) noexcept {
		self.m_min_length = arg;
		return std::forward<decltype(self)>(self);
	}
	auto&& SetMaxLength(this auto&& self, std::uint16_t arg) noexcept {
		self.m_max_length = arg;
		return std::forward<decltype(self)>(self);
	}
};

/** @name JSON value to object conversion
 */
/// @{
eTextInputStyle
tag_invoke(boost::json::value_to_tag<eTextInputStyle>, const boost::json::value&);

cPartialTextInput
tag_invoke(boost::json::value_to_tag<cPartialTextInput>, const boost::json::value&);
/// @}

/** @name JSON value from object conversion
 */
/// @{
void
tag_invoke(boost::json::value_from_tag, boost::json::value&, eTextInputStyle);

void
tag_invoke(boost::json::value_from_tag, boost::json::value&, const cTextInput&);
/// @}
#endif /* DISCORD_TEXTINPUT_H */