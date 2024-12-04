#ifndef DISCORD_CONCEPTS_H
#define DISCORD_CONCEPTS_H
#include <concepts>
#include <type_traits>
#include <utility>

template<typename T>
concept iMutable = !std::is_const_v<std::remove_reference_t<T>>;

template<typename From, typename To>
concept iExplicitlyConvertibleTo = requires { static_cast<To>(std::declval<From>()); };
#endif /* GREEKBOT_CONCEPTS_H */
