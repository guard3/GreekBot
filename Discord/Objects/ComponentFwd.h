#ifndef DISCORD_COMPONENTFWD_H
#define DISCORD_COMPONENTFWD_H
#include "BaseFwd.h"
#include <algorithm>
#include <limits>
#include <variant>

DISCORD_FWDDECL_STRUCT(ActionRow);
DISCORD_FWDDECL_CLASS(Button);
DISCORD_FWDDECL_STRUCT(Label);
DISCORD_FWDDECL_STRUCT(PartialLabel);
DISCORD_FWDDECL_CLASS(PartialTextInput);
DISCORD_FWDDECL_CLASS(SelectMenu);
DISCORD_FWDDECL_CLASS(TextDisplay);
DISCORD_FWDDECL_CLASS(TextInput);
DISCORD_FWDDECL_CLASS(UnsupportedComponent);

// Boost forward declarations
namespace boost::json {
	template<typename T>
	void value_from(T&& t, value& jv);

	template<typename T>
	T value_to(const value& v );
}

enum eComponentType {
	COMPONENT_ACTION_ROW = 1,    // Container to display a row of interactive components
	COMPONENT_BUTTON,            // Button object
	COMPONENT_SELECT_MENU,       // Select menu for picking from defined text options
	COMPONENT_TEXT_INPUT,        // Text input object
	COMPONENT_TEXT_DISPLAY = 10, // Markdown text
	COMPONENT_LABEL = 18,        // Container associating a label and description with a component

	COMPONENT_UNSUPPORTED = std::numeric_limits<int>::max()
};

eComponentType
tag_invoke(boost::json::value_to_tag<eComponentType>, const boost::json::value& v);

void
tag_invoke(boost::json::value_from_tag, boost::json::value&, eComponentType);

template<typename T>
concept iComponent = requires {
	{ T::Type } -> std::same_as<const eComponentType&>;
};

namespace detail {
	template<iComponent... Components>
	constexpr bool is_sorted_component_list() noexcept {
		constexpr eComponentType type_list[] { Components::Type... };
		return std::ranges::is_sorted(type_list) && std::ranges::adjacent_find(type_list) == std::ranges::end(type_list);
	}
}

template<iComponent... ComponentTypes>
requires(detail::is_sorted_component_list<ComponentTypes...>())
struct cVariantComponent : std::variant<ComponentTypes...> {
	using variant_type = std::variant<ComponentTypes...>;
	using variant_type::variant_type;
	using variant_type::operator=;

	template<typename T, typename Self>
	auto&& As(this Self&& self) {
		return std::get<T>(std::forward<Self>(self));
	}

	template<typename T, typename Self>
	auto If(this Self&& self) {
		return cPtr(std::get_if<T>(std::addressof(self)));
	}

	template<typename Self, typename F>
	decltype(auto) Visit(this Self&& self, F&& f) {
		return std::visit(std::forward<F>(f), std::forward<Self>(self));
	}

	template<typename R, typename Self, typename F>
	R Visit(this Self&& self, F&& f) {
		return std::visit<R>(std::forward<F>(f), std::forward<Self>(self));
	}
};

// Disable default JSON conversions
template<iComponent... Components>
struct boost::json::is_variant_like<cVariantComponent<Components...>> : std::false_type {};

// JSON Conversions
template<std::same_as<boost::json::value_from_tag> Tag, iComponent... Components>
void tag_invoke(Tag, boost::json::value& v, const cVariantComponent<Components...>& component) {
	component.Visit([&v](auto& cmp) { boost::json::value_from(cmp, v); });
}

namespace detail {
	[[noreturn]] void throw_variant_exception();
}

template<typename Value, iComponent... Components>
auto tag_invoke(boost::json::value_to_tag<cVariantComponent<Components...>>, const Value& v) {
	using component_type = cVariantComponent<Components...>;

	switch (auto type = boost::json::value_to<eComponentType>(v.at("type"))) {
	case COMPONENT_ACTION_ROW:
		if constexpr (std::is_constructible_v<component_type, std::in_place_type_t<cActionRow>, const Value&>) {
			return component_type(std::in_place_type<cActionRow>, v);
		}
		goto LABEL_UNKNOWN;
	case COMPONENT_BUTTON:
		if constexpr (std::is_constructible_v<component_type, std::in_place_type_t<cButton>, const Value&>) {
			return component_type(std::in_place_type<cButton>, v);
		}
		goto LABEL_UNKNOWN;
	case COMPONENT_SELECT_MENU:
		if constexpr (std::is_constructible_v<component_type, std::in_place_type_t<cSelectMenu>, const Value&>) {
			return component_type(std::in_place_type<cSelectMenu>, v);
		}
		goto LABEL_UNKNOWN;
	case COMPONENT_TEXT_INPUT:
		if constexpr (std::is_constructible_v<component_type, std::in_place_type_t<cTextInput>, const Value&>) {
			return component_type(std::in_place_type<cTextInput>, v);
		} else if constexpr (std::is_constructible_v<component_type, std::in_place_type_t<cPartialTextInput>, const Value&>) {
			return component_type(std::in_place_type<cPartialTextInput>, v);
		}
		goto LABEL_UNKNOWN;
	case COMPONENT_TEXT_DISPLAY:
		if constexpr (std::is_constructible_v<component_type, std::in_place_type_t<cTextDisplay>, const Value&>) {
			return component_type(std::in_place_type<cTextDisplay>, v);
		}
		goto LABEL_UNKNOWN;
	case COMPONENT_LABEL:
		if constexpr (std::is_constructible_v<component_type, std::in_place_type_t<cLabel>, const Value&>) {
			return component_type(std::in_place_type<cLabel>, v);
		} else if constexpr (std::is_constructible_v<component_type, std::in_place_type_t<cPartialLabel>, const Value&>) {
			return component_type(std::in_place_type<cPartialLabel>, v);
		}
	default:
	LABEL_UNKNOWN:
		if constexpr (std::is_constructible_v<component_type, std::in_place_type_t<cUnsupportedComponent>, const Value&>) {
			return component_type(std::in_place_type<cUnsupportedComponent>, v);
		} break;
	}

	detail::throw_variant_exception();
}
#endif //DISCORD_COMPONENTFWD_H
