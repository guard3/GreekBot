#ifndef GREEKBOT_COMPONENT_H
#define GREEKBOT_COMPONENT_H
#include "Emoji.h"
#include <vector>
#include <variant>

enum eComponentType {
	COMPONENT_ACTION_ROW = 1, // A container for other components
	COMPONENT_BUTTON,         // A button object
	COMPONENT_SELECT_MENU,    // A select menu for picking from choices
	COMPONENT_TEXT_INPUT      // A Text input object
};

enum eButtonStyle {
	BUTTON_STYLE_PRIMARY = 1,
	BUTTON_STYLE_SECONDARY,
	BUTTON_STYLE_SUCCESS,
	BUTTON_STYLE_DANGER,
	BUTTON_STYLE_LINK
};

KW_DECLARE(label, std::string)
KW_DECLARE(emoji, cEmoji)
KW_DECLARE(disabled, bool)

class cBaseButton {
private:
	eButtonStyle m_style;
	std::string m_label;
	std::optional<cEmoji> m_emoji;
	std::string m_value; // either url or custom_id depending on button style
	bool m_disabled;

	template<kw::key... Keys>
	cBaseButton(eButtonStyle s, std::string v, kw::pack<Keys...> pack):
		m_style(s),
		m_value(std::move(v)),
		m_label(std::move(kw::get<"label">(pack))),
		m_disabled(kw::get<"disabled">(pack, false)) {
		if (auto p = kw::get_if<"emoji">(pack, kw::nullarg); p)
			m_emoji.emplace(std::move(*p));
	}

protected:
	template<kw::key... Keys>
	cBaseButton(eButtonStyle s, std::string v, kw::arg<Keys>&... kwargs) : cBaseButton(s, std::move(v), kw::pack{ kwargs... }) {}

	std::string&       get_value()       noexcept { return m_value; }
	const std::string& get_value() const noexcept { return m_value; }

public:
	/* Non const getters */
	std::string& GetLabel() noexcept { return m_label; }
	hEmoji GetEmoji() noexcept { return m_emoji ? &m_emoji.value() : nullptr; }
	/* Const getters */
	eButtonStyle GetStyle() const noexcept { return m_style; }
	const std::string& GetLabel() const noexcept { return m_label; }
	chEmoji GetEmoji() const noexcept { return const_cast<cBaseButton*>(this)->GetEmoji(); }
	bool IsDisabled() const noexcept { return m_disabled; }

	json::object ToJson() const;
};

template<eButtonStyle s>
class cButton final : public cBaseButton {
public:
	template<kw::key Key, kw::key... Keys>
	cButton(std::string custom_id, kw::arg<Key>& kwarg, kw::arg<Keys>&... kwargs) : cBaseButton(s, std::move(custom_id), kwarg, kwargs...) {}

	std::string&       GetCustomId()       noexcept { return get_value(); }
	const std::string& GetCustomId() const noexcept { return get_value(); }
};

template<>
class cButton<BUTTON_STYLE_LINK> final : public cBaseButton {
public:
	template<kw::key Key, kw::key... Keys>
	cButton(std::string url, kw::arg<Key>& kwarg, kw::arg<Keys>&... kwargs) : cBaseButton(BUTTON_STYLE_LINK, std::move(url), kwarg, kwargs...) {}

	std::string&       GetUrl() noexcept { return get_value(); }
	const std::string& GetUrl() const noexcept { return get_value(); }
};

class cSelectOption final {
private:
	std::string           m_label;
	std::string           m_value;
	std::string           m_description;
	std::optional<cEmoji> m_emoji;

	template<kw::key... Keys>
	cSelectOption(std::string&& l, std::string&& v, kw::pack<Keys...> pack):
		m_label(std::move(l)),
		m_value(std::move(v)),
		m_description(std::move(kw::get<"description">(pack))) {
		if (auto p = kw::get_if<"emoji">(pack, kw::nullarg))
			m_emoji.emplace(std::move(*p));
	}

public:
	template<kw::key... Keys>
	cSelectOption(std::string label, std::string value, kw::arg<Keys>&... kwargs) : cSelectOption(std::move(label), std::move(value), kw::pack{ kwargs... }) {}

	json::object ToJson() const;
};

class cSelectMenu final {
private:
	std::string custom_id;
	std::string placeholder;
	std::vector<cSelectOption> options;

public:
	template<typename... Args>
	explicit cSelectMenu(const char* custom_id, Args... opts) : custom_id(custom_id ? custom_id : std::string()), options{ std::move(opts)... } {}
	template<typename... Args>
	cSelectMenu(const char* custom_id, const char* placeholder, Args... opts) : cSelectMenu(custom_id, opts...) { if (placeholder) this->placeholder = placeholder; }

	json::object ToJson() const;
};

enum eTextInputStyle {
	TEXT_INPUT_SHORT = 1,
	TEXT_INPUT_PARAGRAPH,
};

class cTextInput final {
private:
	std::string m_custom_id;
	std::string m_label;
	eTextInputStyle m_style;

public:
	cTextInput(eTextInputStyle style, std::string custom_id, std::string label):
		m_custom_id(std::move(custom_id)),
		m_label(std::move(label)),
		m_style(style) {}

	json::object ToJson() const;
};

class cComponent final {
private:
	std::variant<
	    cButton<BUTTON_STYLE_LINK>,
		cButton<BUTTON_STYLE_DANGER>,
	    cButton<BUTTON_STYLE_PRIMARY>,
		cButton<BUTTON_STYLE_SECONDARY>,
		cButton<BUTTON_STYLE_SUCCESS>,
		cSelectMenu,
		cTextInput> m_component;

	json::object (*m_func)(const void*);

public:
	template<eButtonStyle e>
	cComponent(cButton<e>);
	cComponent(cSelectMenu);
	cComponent(cTextInput);

	json::object ToJson() const;
};

class cActionRow {
private:
	const std::vector<cComponent> m_components;

public:
	template<typename... Args, typename = std::enable_if_t<(sizeof...(Args) < 6)>>
	explicit cActionRow(Args&&... buttons)     : m_components{ std::forward<Args>(buttons)... } {}
	explicit cActionRow(const cSelectMenu&  m) : m_components{ m } {}
	explicit cActionRow(      cSelectMenu&& m) : m_components{ std::forward<cSelectMenu>(m) } {}

	const std::vector<cComponent>& GetComponents() const noexcept { return m_components; }
};
typedef   hHandle<cActionRow>   hActionRow;
typedef  chHandle<cActionRow>  chActionRow;
typedef  uhHandle<cActionRow>  uhActionRow;
typedef uchHandle<cActionRow> uchActionRow;
typedef  shHandle<cActionRow>  shActionRow;
typedef schHandle<cActionRow> schActionRow;

void tag_invoke(const json::value_from_tag&, json::value&, const cActionRow&);

KW_DECLARE(components, std::vector<cActionRow>)
#endif //GREEKBOT_COMPONENT_H