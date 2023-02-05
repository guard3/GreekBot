#ifndef GREEKBOT_KWARG_H
#define GREEKBOT_KWARG_H
#include <utility>
#include <optional>
#include <type_traits>
/* ========== The 'nil' keyword as null parameter for kwargs ======================================================== */
struct nil_t { explicit constexpr nil_t() = default; };
inline constexpr nil_t nil;
/* ================================================================================================================== */
namespace detail {
	struct kwtag {};
	template<typename T>
	concept iTag = std::is_base_of_v<kwtag, T>;

	template<iTag>
	struct kwdetail;
}
using detail::iTag;
template<iTag T>
using tKwValue = typename detail::kwdetail<T>::value_type;
template<iTag T>
using tKwOptValue = std::optional<tKwValue<T>>;
/* ================================================================================================================== */
template<iTag T>
class cKwArg {
public:
	using tTag   = T;              // The tag type
	using tValue = tKwValue<tTag>; // The enclosing value type
	/* Disable copy constructor */
	cKwArg(const cKwArg&) = delete;
	cKwArg& operator=(const cKwArg&) = delete;
	/* Value assignment */
	template<typename U> requires std::is_same_v<std::decay_t<U>, nil_t>
	cKwArg& operator=(U&&) {
		m_opt.reset();
		return *this;
	}
	template<typename U = tValue> requires std::is_assignable_v<tValue&, U&&>
	cKwArg& operator=(U&& u) {
		m_opt = std::forward<U>(u);
		return *this;
	}

private:
	/* Make the constructor private */
	explicit cKwArg(nil_t) {}
	/* The value */
	std::optional<tValue> m_opt;
	/* Friends */
	friend struct detail::kwdetail<tTag>;
	template<iTag...> friend class cKwPack;
};
/* ========== The kwarg concept ===================================================================================== */

/* ========== Default empty kwarg pack ============================================================================== */
template<iTag... Tags>
class cKwPack {
private:
	template<iTag T>
	static tKwOptValue<T>& get_optional() { return detail::kwdetail<T>::instance.m_opt; }

public:
	explicit cKwPack(cKwArg<Tags>&...) {}

	template<iTag T, typename Arg = tKwValue<T>> requires std::is_assignable_v<cKwArg<T>&, Arg&&>
	friend tKwOptValue<T>& KwOptGet(cKwPack&, Arg&& arg) {
		detail::kwdetail<T>::instance = std::forward<Arg>(arg);
		return get_optional<T>();
	}
	template<iTag T, typename... Args> requires std::is_constructible_v<tKwValue<T>, Args&&...>
	friend tKwOptValue<T>& KwOptGet(cKwPack&, Args&&... args) {
		get_optional<T>().emplace(std::forward<Args>(args)...);
		return get_optional<T>();
	}
};
/* ========== Non-empty kwarg pack specialization =================================================================== */
template<iTag First, iTag... Rest>
class cKwPack<First, Rest...> {
private:
	cKwPack<Rest...> m_rest;

	template<iTag T>
	static tKwOptValue<T>& get_optional() noexcept { return detail::kwdetail<T>::instance.m_opt; }

public:
	explicit cKwPack(cKwArg<First>&, cKwArg<Rest>&... r) : m_rest(r...) {}

	template<iTag T, typename Arg = tKwValue<T>> requires std::is_assignable_v<cKwArg<T>&, Arg&&>
	friend tKwOptValue<T>& KwOptGet(cKwPack pack, Arg&& arg) {
		if constexpr (std::is_same_v<T, First>)
			return get_optional<T>();
		else
			return KwOptGet<T>(pack.m_rest, std::forward<Arg>(arg));
	}
	template<iTag T, typename... Args> requires std::is_constructible_v<tKwValue<T>, Args&&...>
	friend tKwOptValue<T>& KwOptGet(cKwPack pack, Args&&... args) {
		if constexpr (std::is_same_v<T, First>)
			return get_optional<T>();
		else
			return KwOptGet<T>(pack.m_rest, std::forward<Args>(args)...);
	}
};
namespace detail {
	template<typename>
	struct is_kwarg : std::false_type {};
	template<iTag T>
	struct is_kwarg<cKwArg<T>> : std::true_type {};
	template<typename>
	struct is_kwpack : std::false_type {};
	template<iTag... T>
	struct is_kwpack<cKwPack<T...>> : std::true_type {};
};
template<typename T>
concept iKwArg = detail::is_kwarg<std::decay_t<T>>::value;
template<typename T>
concept iKwPack = detail::is_kwpack<std::decay_t<T>>::value;
/* ================================================================================================================== */
template<iTag T, iTag... Tags, typename Arg = tKwValue<T>> requires std::is_assignable_v<cKwArg<T>&, Arg&&>
inline tKwOptValue<T> KwOptMove(cKwPack<Tags...> pack, Arg&& arg) {
	return std::move(KwOptGet<T>(pack, std::forward<Arg>(arg)));
}
template<iTag T, iTag... Tags, typename... Args> requires std::is_constructible_v<tKwValue<T>, Args&&...>
inline tKwOptValue<T> KwOptMove(cKwPack<Tags...> pack, Args&&... args) {
	return std::move(KwOptGet<T>(pack, std::forward<Args>(args)...));
}
template<iTag T>
inline tKwOptValue<T> KwOptMove(cKwPack<>, nil_t) {
	return {};
}
template<iTag T, typename Arg = tKwValue<T>> requires std::is_constructible_v<tKwValue<T>, Arg&&>
inline tKwOptValue<T> KwOptMove(cKwPack<>, Arg&& arg) {
	return { std::in_place, std::forward<Arg>(arg) };
}
template<iTag T, typename... Args> requires std::is_constructible_v<tKwValue<T>, Args&&...>
inline tKwOptValue<T> KwOptMove(cKwPack<>, Args&&... args) {
	return { std::in_place, std::forward<Args>(args)... };
}
/* ================================================================================================================== */
template<iTag T, iTag... Tags, typename Arg = tKwValue<T>> requires std::is_constructible_v<tKwValue<T>, Arg&&>
inline tKwValue<T>& KwGet(cKwPack<Tags...> pack, Arg&& arg) {
	return *KwOptGet<T>(pack, std::forward<Arg>(arg));
}
template<iTag T, iTag... Tags, typename... Args> requires std::is_constructible_v<tKwValue<T>, Args&&...>
inline tKwValue<T>& KwGet(cKwPack<Tags...> pack, Args&&... args) {
	return *KwOptGet<T>(pack, std::forward<Args>(args)...);
}
/* ================================================================================================================== */
template<iTag T, iTag... Tags, typename Arg = tKwValue<T>> requires std::is_constructible_v<tKwValue<T>, Arg&&>
inline tKwValue<T> KwMove(cKwPack<Tags...> pack, Arg&& arg) {
	return std::move(KwGet<T>(pack, std::forward<Arg>(arg)));
}
template<iTag T, iTag... Tags, typename... Args> requires std::is_constructible_v<tKwValue<T>, Args&&...>
inline tKwValue<T> KwMove(cKwPack<Tags...> pack, Args&&... args) {
	return std::move(KwGet<T>(pack, std::forward<Args>(args)...));
}
template<iTag T>
inline tKwValue<T> KwMove(cKwPack<>, tKwValue<T> v) { return v; }
template<iTag T, typename... Args> requires std::is_constructible_v<tKwValue<T>, Args&&...>
inline tKwValue<T> KwMove(cKwPack<>, Args&&... args) { return tKwValue<T>(std::forward<Args>(args)...); }
/*
 * KWARG_DECLARE():
 * - First create the tag type
 * - Then specialize kwdetail so that tKwValue<> works
 * - Then initialize the static instance of the kwarg
 * - Finally, create a reference with the specified name
 */
#define KW_DECLARE(name, tag, type)\
struct tag : detail::kwtag {};\
template<> struct detail::kwdetail<tag> { static cKwArg<tag> instance; using value_type = type; };\
inline cKwArg<tag> detail::kwdetail<tag>::instance{nil};\
namespace kw { inline cKwArg<tag>& name = detail::kwdetail<tag>::instance; }
#endif //GREEKBOT_KWARG_H