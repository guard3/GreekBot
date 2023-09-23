#ifndef GREEKBOT_TEXTINPUT_H
#define GREEKBOT_TEXTINPUT_H
#include "Common.h"

KW_DECLARE(min_length, int)
KW_DECLARE(max_length, int)

enum eTextInputStyle {
	TEXT_INPUT_SHORT = 1,
	TEXT_INPUT_PARAGRAPH,
};

class cTextInput final {
private:
	std::string m_custom_id;
	std::string m_label;
	eTextInputStyle m_style;
	int m_min_length;
	int m_max_length;

	template<kw::key... Keys, typename S1, typename S2>
	cTextInput(eTextInputStyle style, S1&& custom_id, S2&& label, kw::pack<Keys...> pack):
		m_custom_id(std::forward<S1>(custom_id)),
		m_label(std::forward<S2>(label)),
		m_style(style),
		m_min_length(kw::get<"min_length">(pack, 0)),
		m_max_length(kw::get<"max_length">(pack, 4000)) {}

public:
	template<kw::key... Keys, typename Str1 = std::string, typename Str2 = std::string>
	requires std::constructible_from<std::string, Str1&&> && std::constructible_from<std::string, Str2&&>
	cTextInput(eTextInputStyle style, Str1&& custom_id, Str2&& label, kw::arg<Keys>&... kwargs): cTextInput(style, std::forward<Str1>(custom_id), std::forward<Str2>(label), kw::pack{ kwargs... }) {}

	friend void tag_invoke(const json::value_from_tag&, json::value&, const cTextInput&);
};

void tag_invoke(const json::value_from_tag&, json::value&, const cTextInput&);
#endif /* GREEKBOT_TEXTINPUT_H */