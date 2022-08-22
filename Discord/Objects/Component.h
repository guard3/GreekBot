#pragma once
#ifndef _GREEKBOT_COMPONENT_H_
#define _GREEKBOT_COMPONENT_H_
#include "Common.h"
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
	eComponentType type;
public:
	explicit cComponent(eComponentType type) : type(type) {}
	explicit cComponent(const json::value& v) : cComponent(static_cast<eComponentType>(v.at("type").as_int64())) {}
	virtual ~cComponent() = default;

	eComponentType GetType() const { return type; }

	virtual json::object ToJson() const {
		json::object obj;
		obj["type"] = static_cast<int>(type);
		return obj;
	};
};
typedef   hHandle<cComponent>   hComponent;
typedef  chHandle<cComponent>  chComponent;
typedef  uhHandle<cComponent>  uhComponent;
typedef uchHandle<cComponent> uchComponent;
typedef  shHandle<cComponent>  shComponent;
typedef schHandle<cComponent> schComponent;

class cBaseButton : public cComponent {
private:
	eButtonStyle style;
	std::string label;
	// emoji
	std::string custom_id;
	std::string url;
	// disabled

public:
	cBaseButton(eButtonStyle s, const char* l, const char* c, const char* u) : cComponent(COMPONENT_BUTTON), style(s), label(l ? l : std::string()), custom_id(c ? c : std::string()), url(u ? u : std::string()) {}
	~cBaseButton() override = default;

	[[nodiscard]] eButtonStyle GetStyle() const { return style; }

	[[nodiscard]] json::object ToJson() const override {
		json::object obj = cComponent::ToJson();
		obj["style"] = static_cast<int>(style);
		if (!label.empty())
			obj["label"] = label;
		if (!custom_id.empty())
			obj["custom_id"] = custom_id;
		if (!url.empty())
			obj["url"] = url;
		return obj;
	}
};

template<eButtonStyle s>
class cButton final : public cBaseButton {
public:
	explicit cButton(const char* custom_id, const char* label = nullptr) : cBaseButton(s, label, custom_id, nullptr) {}
};

template<>
class cButton<BUTTON_STYLE_LINK> final : public cBaseButton {
public:
	explicit cButton(const char* url, const char* label = nullptr) : cBaseButton(BUTTON_STYLE_LINK, label, nullptr, url) {}
};

class cSelectOption final {
private:
	std::string label;
	std::string value;
	std::string description;
	chEmoji     emoji;

public:
	cSelectOption(const char* label, const char* value, const char* description = nullptr) : label(label), value(value), description(description ? description : std::string()), emoji(nullptr) {}
	cSelectOption(const char* label, const char* value, const cEmoji&  emoji) : label(label), value(value), emoji(new cEmoji(emoji)) {}
	cSelectOption(const char* label, const char* value,       cEmoji&& emoji) : label(label), value(value), emoji(new cEmoji(std::forward<cEmoji>(emoji))) {}
	cSelectOption(const char* label, const char* value, const char* description, const cEmoji&  emoji) : label(label), value(value), description(description), emoji(new cEmoji(emoji)) {}
	cSelectOption(const char* label, const char* value, const char* description,       cEmoji&& emoji) : label(label), value(value), description(description), emoji(new cEmoji(std::forward<cEmoji>(emoji))) {}

	cSelectOption(const cSelectOption& o) : label(o.label), value(o.value), description(o.description), emoji(o.emoji ? new cEmoji(*o.emoji) : nullptr) {}
	cSelectOption(cSelectOption&& o) noexcept : label(std::move(o.label)), value(std::move(o.value)), description(std::move(o.description)), emoji(o.emoji) { o.emoji = nullptr; }
	~cSelectOption() { delete emoji; }

	cSelectOption& operator=(cSelectOption o) {
		label = std::move(o.label);
		value = std::move(o.value);
		description = std::move(o.description);
		emoji = o.emoji;
		o.emoji = nullptr;
		return *this;
	}

	[[nodiscard]] json::value ToJson() const {
		json::object obj;
		obj["label"] = label;
		obj["value"] = value;
		if (!description.empty())
			obj["description"] = description;
		if (emoji)
			obj["emoji"] = emoji->ToJson();
		return obj;
	}
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

	[[nodiscard]] json::object ToJson() const override {
		json::object obj = cComponent::ToJson();
		if (!custom_id.empty())
			obj["custom_id"] = custom_id;
		if (!placeholder.empty())
			obj["placeholder"] = placeholder;
		if (!options.empty()) {
			json::array opts;
			opts.reserve(options.size());
			for (auto& o : options)
				opts.push_back(o.ToJson());
			obj["options"] = std::move(opts);
		}
		return obj;
	}
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

	[[nodiscard]] json::object ToJson() const override {
		json::object obj = cComponent::ToJson();
		if (!Components.empty()) {
			json::array components;
			components.reserve(Components.size());
			for (chComponent c : Components)
				components.push_back(c->ToJson());
			obj["components"] = std::move(components);
		}
		return obj;
	}
};
typedef   hHandle<cActionRow>   hActionRow;
typedef  chHandle<cActionRow>  chActionRow;
typedef  uhHandle<cActionRow>  uhActionRow;
typedef uchHandle<cActionRow> uchActionRow;
typedef  shHandle<cActionRow>  shActionRow;
typedef schHandle<cActionRow> schActionRow;

#endif /* _GREEKBOT_COMPONENT_H_ */
