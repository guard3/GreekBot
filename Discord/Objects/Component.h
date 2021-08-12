#pragma once
#ifndef _GREEKBOT_COMPONENT_H_
#define _GREEKBOT_COMPONENT_H_
#include "Types.h"

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

	[[nodiscard]] eComponentType GetType() const { return type; }

	[[nodiscard]] virtual json::object ToJson() const {
		json::object obj;
		obj["type"] = static_cast<int>(type);
		return obj;
	};
	[[nodiscard]] std::string ToJsonString() const { return (std::stringstream() << ToJson()).str(); }
};
typedef   hHandle<cComponent>   hComponent;
typedef  chHandle<cComponent>  chComponent;
typedef  uhHandle<cComponent>  uhComponent;
typedef uchHandle<cComponent> uchComponent;
typedef  shHandle<cComponent>  shComponent;
typedef schHandle<cComponent> schComponent;

class _Button : public cComponent {
protected:
	eButtonStyle style;
	std::string label;
	// emoji
	std::string custom_id;
	std::string url;
	// disabled

public:
	_Button(eButtonStyle s) : cComponent(COMPONENT_BUTTON), style(s) {}
	_Button(eButtonStyle s, const std::string& label) : cComponent(COMPONENT_BUTTON), style(s), label(label) {}

	[[nodiscard]] eButtonStyle GetStyle() const { return style; }

	[[nodiscard]] json::object ToJson() const override {
		json::object obj;
		obj["type"] = static_cast<int>(GetType());
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
class cButton final : public _Button {
public:
	cButton(const std::string& custom_id) : _Button(s) { this->custom_id = custom_id;}
	cButton(const std::string& custom_id, const std::string& label) : cButton(custom_id) { this->label = label; }
};

template<>
class cButton<BUTTON_STYLE_LINK> final : public _Button {
public:
	cButton(const std::string& url) : _Button(BUTTON_STYLE_LINK) { this->url = url; }
	cButton(const std::string& url, const std::string& label) : cButton(url) { this->label = label; }
};

class cSelectOption final {
private:
	std::string label;
	std::string value;
	std::string description;
	// emoji

public:
	cSelectOption(const char* label, const char* value, const char* description = nullptr) :
	label(label ? label : std::string()),
	value(value ? value : std::string()),
	description(description ? description : std::string()) {}

	[[nodiscard]] json::value ToJson() const {
		json::object obj;
		if (!label.empty())
			obj["label"] = label;
		if (!value.empty())
			obj["value"] = value;
		if (!description.empty())
			obj["description"] = description;
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
	explicit cSelectMenu(const char* custom_id, Args... opts) : cComponent(COMPONENT_SELECT_MENU), custom_id(custom_id ? custom_id : std::string()), options{ opts... } {}
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
private:
	void populate_components() {}
	template<eButtonStyle s, typename... Args>
	void populate_components(const cButton<s>& b, Args... bb) {
		const_cast<std::vector<chComponent>&>(Components).push_back(new cButton<s>(b));
		populate_components(bb...);
	}
	template<eButtonStyle s, typename... Args>
	void populate_components(cButton<s>&& b, Args... bb) {
		const_cast<std::vector<chComponent>&>(Components).push_back(new cButton<s>(std::move(b)));
		populate_components(bb...);
	}

public:
	const std::vector<chComponent> Components;

	template<typename... Args, typename = std::enable_if_t<(sizeof...(Args) < 6)>>
	explicit cActionRow(Args... buttons) : cComponent(COMPONENT_ACTION_ROW) { populate_components(buttons...); }
	explicit cActionRow(const cSelectMenu&  m) : cComponent(COMPONENT_ACTION_ROW), Components{ new cSelectMenu(m) } {}
	explicit cActionRow(      cSelectMenu&& m) : cComponent(COMPONENT_ACTION_ROW), Components{ new cSelectMenu(m) } {}

	cActionRow(const cActionRow& o) : cComponent(o.GetType()) {
		auto& components = const_cast<std::vector<chComponent>&>(Components);
		components.reserve(o.Components.size());
		for (chComponent c : o.Components) {
			switch (c->GetType()) {
				case COMPONENT_ACTION_ROW:
					components.push_back(new cActionRow(*dynamic_cast<const cActionRow*>(c)));
					break;
				case COMPONENT_BUTTON:
					components.push_back(new _Button(*dynamic_cast<const _Button*>(c)));
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
