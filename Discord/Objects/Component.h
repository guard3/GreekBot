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
#include <iostream>
class cComponent {
private:
	eComponentType type;
public:
	explicit cComponent(eComponentType type) : type(type) {}
	explicit cComponent(const json::value& v) : cComponent(static_cast<eComponentType>(v.at("type").as_int64())) {}
	virtual ~cComponent() = default;

	[[nodiscard]] eComponentType GetType() const { return type; }

	[[nodiscard]] virtual json::value ToJson() const {
		json::object obj;
		obj["type"] = static_cast<int>(type);
		return obj;
	};
	[[nodiscard]] virtual std::string ToJsonString() const { return (std::stringstream() << ToJson()).str(); }
};
typedef   hHandle<cComponent>   hComponent;
typedef  chHandle<cComponent>  chComponent;
typedef  uhHandle<cComponent>  uhComponent;
typedef uchHandle<cComponent> uchComponent;
typedef  shHandle<cComponent>  shComponent;
typedef schHandle<cComponent> schComponent;

class cButtonw : public cComponent {
public:
	cButtonw() : cComponent(COMPONENT_BUTTON) {}
};

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

	[[nodiscard]] json::value ToJson() const override {
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
	[[nodiscard]] std::string ToJsonString() const override { return (std::stringstream() << ToJson()).str(); }
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

class cSelectMenu final : public cComponent {
public:
	cSelectMenu() : cComponent(COMPONENT_SELECT_MENU) {}
};

class cActionRow : public cComponent {
private:
	template<eButtonStyle s>
	void add_components(const cButton<s>& component) {
		const_cast<std::vector<chComponent>&>(Components).push_back(new cButton(component));
	}
	void add_components(const cSelectMenu& component) {
		const_cast<std::vector<chComponent>&>(Components).push_back(new cSelectMenu(component));
	}
	template<eButtonStyle s, typename... Args>
	void add_components(const cButton<s>& component, const Args&... components) {
		add_components(component);
		add_components(components...);
	}
	template<typename... Args>
	void add_components(const cSelectMenu& component, const Args&... components) {
		add_components(component);
		add_components(components...);
	}

public:
	const std::vector<chComponent> Components;

	cActionRow() : cComponent(COMPONENT_ACTION_ROW) {}
	template<typename... Args>
	explicit cActionRow(const Args&... components) : cActionRow() {
		const_cast<std::vector<chComponent>&>(Components).reserve(sizeof...(components));
		add_components(components...);
	}
	cActionRow(const cActionRow& o) : cComponent(o.GetType()) {
		auto& c = const_cast<std::vector<chComponent>&>(Components);
		c.reserve(o.Components.size());
		for (chComponent p : c) {
			switch (p->GetType()) {
				case COMPONENT_ACTION_ROW:
					c.push_back(new cActionRow(*dynamic_cast<const cActionRow*>(p)));
				case COMPONENT_BUTTON:
				case COMPONENT_SELECT_MENU:
					// TODO: TBA
					break;
			}
		}
	}
	cActionRow(cActionRow&& o)  noexcept : cComponent(o.GetType()), Components(std::move(const_cast<std::vector<chComponent>&>(o.Components))) {}
	~cActionRow() override { for (chComponent p : Components) delete p; }

	cActionRow& operator=(cActionRow o) {
		const_cast<std::vector<chComponent>&>(Components) = std::move(const_cast<std::vector<chComponent>&>(o.Components));
		return *this;
	}

	template<typename Arg>
	cActionRow& AddComponent(const Arg& component) {
		add_components(component);
		return *this;
	}
	template<typename... Args>
	cActionRow& AddComponents(const Args&... components) {
		add_components(components...);
		return *this;
	}

	[[nodiscard]] json::value ToJson() const override {
		json::object obj;
		obj["type"] = static_cast<int>(GetType());
		if (!Components.empty()) {
			json::array components;
			components.reserve(Components.size());
			for (chComponent p : Components)
				components.push_back(p->ToJson());
			obj["components"] = components;
		}
		return obj;
	}

	[[nodiscard]] std::string ToJsonString() const override { return (std::stringstream() << ToJson()).str(); }
};
typedef   hHandle<cActionRow>   hActionRow;
typedef  chHandle<cActionRow>  chActionRow;
typedef  uhHandle<cActionRow>  uhActionRow;
typedef uchHandle<cActionRow> uchActionRow;
typedef  shHandle<cActionRow>  shActionRow;
typedef schHandle<cActionRow> schActionRow;

#endif /* _GREEKBOT_COMPONENT_H_ */
