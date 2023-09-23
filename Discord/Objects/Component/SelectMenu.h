#ifndef GREEKBOT_SELECTMENU_H
#define GREEKBOT_SELECTMENU_H
#include "Common.h"
#include "Emoji.h"

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
		if (auto p = kw::get_if<"emoji">(pack, kw::nullarg))
			m_emoji.emplace(std::move(*p));
	}

public:
	template<kw::key... Keys, typename Str1 = std::string, typename Str2 = std::string>
	requires std::constructible_from<std::string, Str1&&> && std::constructible_from<std::string, Str2&&>
	cSelectOption(Str1&& label, Str2&& value, kw::arg<Keys>&... kwargs) : cSelectOption(std::forward<Str1>(label), std::forward<Str2>(value), kw::pack{ kwargs... }) {}

	friend void tag_invoke(const json::value_from_tag&, json::value&, const cSelectOption&);
};

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
	template<kw::key... Keys, typename Str = std::string> requires std::constructible_from<std::string, Str&&>
	cSelectMenu(Str&& custom_id, std::vector<cSelectOption> options, kw::arg<Keys>&... kwargs): cSelectMenu(std::forward<Str>(custom_id), std::move(options), kw::pack{ kwargs... }) {}

	friend void tag_invoke(const json::value_from_tag&, json::value&, const cSelectMenu&);
};

void tag_invoke(const json::value_from_tag&, json::value&, const cSelectMenu&);
#endif /* GREEKBOT_SELECTMENU_H */
