#ifndef GREEKBOT_COMPONENT_H
#define GREEKBOT_COMPONENT_H
#include "Emoji.h"

enum eComponentType {
	COMPONENT_ACTION_ROW = 1, // A container for other components
	COMPONENT_BUTTON,         // A button object
	COMPONENT_SELECT_MENU     // A select menu for picking from choices
};

enum eButtonStyle {
	BUTTON_STYLE_PRIMARY = 1,
	BUTTON_STYLE_SECONDARY,
	BUTTON_STYLE_SUCCESS,
	BUTTON_STYLE_DANGER,
	BUTTON_STYLE_LINK
};

class cComponent {
private:
	eComponentType m_type;
public:
	explicit cComponent(eComponentType type) : m_type(type) {}
	explicit cComponent(const json::object&);
	explicit cComponent(const json::value&);
	virtual ~cComponent() = default;

	eComponentType GetType() const noexcept { return m_type; }

	virtual json::object ToJson() const;
};
typedef   hHandle<cComponent>   hComponent;
typedef  chHandle<cComponent>  chComponent;
typedef  uhHandle<cComponent>  uhComponent;
typedef uchHandle<cComponent> uchComponent;
typedef  shHandle<cComponent>  shComponent;
typedef schHandle<cComponent> schComponent;

KW_DECLARE(label, KW_LABEL, std::string)
KW_DECLARE(emoji, KW_EMOJI, cEmoji)
KW_DECLARE(disabled, KW_DISABLED, bool)

class cBaseButton : public cComponent {
private:
	eButtonStyle m_style;
	std::string m_label;
	std::optional<cEmoji> m_emoji;
	std::string m_value; // either url or custom_id depending on button style
	bool m_disabled;

	template<typename String>
	cBaseButton(eButtonStyle s, String&& v, iKwPack auto&& pack):
		cComponent(COMPONENT_BUTTON),
		m_style(s),
		m_value(std::forward<String>(v)),
		m_label(KwMove<KW_LABEL>(pack)),
		m_emoji(KwOptMove<KW_EMOJI>(pack, nil)),
		m_disabled(KwGet<KW_DISABLED>(pack, false)) {}

protected:
	template<typename String>
	cBaseButton(eButtonStyle s, String&& v, iKwArg auto&... kwargs) : cBaseButton(s, std::forward<String>(v), cKwPack(kwargs...)) {}
	~cBaseButton() override = default;

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

	json::object ToJson() const override;
};

template<eButtonStyle s>
class cButton final : public cBaseButton {
public:
	template<typename String>
	cButton(String&& custom_id, iKwArg auto& kwarg, iKwArg auto&... kwargs) : cBaseButton(s, std::forward<String>(custom_id), kwarg, kwargs...) {}

	std::string&       GetCustomId()       noexcept { return get_value(); }
	const std::string& GetCustomId() const noexcept { return get_value(); }
};

template<>
class cButton<BUTTON_STYLE_LINK> final : public cBaseButton {
public:
	template<typename String>
	cButton(String&& url_, iKwArg auto& kwarg, iKwArg auto&... kwargs) : cBaseButton(BUTTON_STYLE_LINK, std::forward<String>(url_), kwarg, kwargs...) {}

	std::string&       GetUrl() noexcept { return get_value(); }
	const std::string& GetUrl() const noexcept { return get_value(); }
};

class cSelectOption final {
private:
	std::string           m_label;
	std::string           m_value;
	std::string           m_description;
	std::optional<cEmoji> m_emoji;

	cSelectOption(std::string&& l, std::string&& v, iKwPack auto pack):
		m_label(std::forward<std::string>(l)),
		m_value(std::forward<std::string>(v)),
		m_description(KwMove<KW_DESCRIPTION>(pack)),
		m_emoji(KwOptMove<KW_EMOJI>(pack, nil)) {}

public:
	cSelectOption(std::string label, std::string value, iKwArg auto&... kwargs) : cSelectOption(std::move(label), std::move(value), cKwPack(kwargs...)) {}

	json::object ToJson() const;
};

class cSelectMenu final : public cComponent {
private:
	std::string custom_id;
	std::string placeholder;
	std::vector<cSelectOption> options;

public:
	template<typename... Args>
	explicit cSelectMenu(const char* custom_id, Args... opts) : cComponent(COMPONENT_SELECT_MENU), custom_id(custom_id ? custom_id : std::string()), options{ std::move(opts)... } {}
	template<typename... Args>
	cSelectMenu(const char* custom_id, const char* placeholder, Args... opts) : cSelectMenu(custom_id, opts...) { if (placeholder) this->placeholder = placeholder; }

	~cSelectMenu() override = default;

	json::object ToJson() const override;
};

class cActionRow : public cComponent {
public:
	const std::vector<chComponent> Components;

	template<typename... Args, typename = std::enable_if_t<(sizeof...(Args) < 6)>>
	explicit cActionRow(Args... buttons)       : cComponent(COMPONENT_ACTION_ROW), Components{ new cBaseButton(std::move(buttons))... } {}
	explicit cActionRow(const cSelectMenu&  m) : cComponent(COMPONENT_ACTION_ROW), Components{ new cSelectMenu(m) } {}
	explicit cActionRow(      cSelectMenu&& m) : cComponent(COMPONENT_ACTION_ROW), Components{ new cSelectMenu(std::forward<cSelectMenu>(m)) } {}

	cActionRow(const cActionRow& o) : cComponent(o.GetType()) {
		auto& components = const_cast<std::vector<chComponent>&>(Components);
		components.reserve(o.Components.size());
		for (chComponent c : o.Components) {
			switch (c->GetType()) {
				case COMPONENT_ACTION_ROW:
					components.push_back(new cActionRow(*dynamic_cast<const cActionRow*>(c)));
					break;
				case COMPONENT_BUTTON:
					components.push_back(new cBaseButton(*dynamic_cast<const cBaseButton*>(c)));
					break;
				case COMPONENT_SELECT_MENU:
					components.push_back(new cSelectMenu(*dynamic_cast<const cSelectMenu*>(c)));
					break;
			}
		}
	}
	cActionRow(cActionRow&& o) noexcept : cComponent(o.GetType()), Components(std::move(const_cast<std::vector<chComponent>&>(o.Components))) {}
	~cActionRow() override { for (chComponent c : Components) delete c; }

	cActionRow& operator=(cActionRow o) {
		const_cast<std::vector<chComponent>&>(Components) = std::move(const_cast<std::vector<chComponent>&>(o.Components));
		return *this;
	}

	json::object ToJson() const override;
};
typedef   hHandle<cActionRow>   hActionRow;
typedef  chHandle<cActionRow>  chActionRow;
typedef  uhHandle<cActionRow>  uhActionRow;
typedef uchHandle<cActionRow> uchActionRow;
typedef  shHandle<cActionRow>  shActionRow;
typedef schHandle<cActionRow> schActionRow;

KW_DECLARE(components, KW_COMPONENTS, std::vector<cActionRow>)

#endif //GREEKBOT_COMPONENT_H