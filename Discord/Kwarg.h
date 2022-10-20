#ifndef GREEKBOT_KWARG_H
#define GREEKBOT_KWARG_H
#include <utility>
#include <type_traits>

template<int> class cKwArg;

namespace detail::kwarg {
	template<int> struct kwtraits;
	template<int key> using tKwType = typename kwtraits<key>::value_type;

	template<typename> struct is_kwarg : std::false_type {};
	template<int key> struct is_kwarg<cKwArg<key>> : std::true_type {};
	template<typename T> concept iKwArg = is_kwarg<T>::value || is_kwarg<std::remove_cvref_t<T>>::value;
}
using detail::kwarg::tKwType;
using detail::kwarg::iKwArg;

template<iKwArg...> class cKwPack;

template<int k>
class cKwArg final {
public:
	static constexpr int Key = k;
	static cKwArg Instance;
	/* The enclosing value type */
	typedef tKwType<k> tValueType;
	/* Constructor */
	cKwArg(const cKwArg&) = delete;
	cKwArg& operator=(const cKwArg&) = delete;

	cKwArg& operator=(const tValueType& t) {
		m_value = t;
		return *this;
	}
	cKwArg& operator=(tValueType&& t) {
		m_value = std::forward<tValueType>(t);
		return *this;
	}
	template<typename U>
	cKwArg& operator=(std::initializer_list<U> list) {
		m_value = list;
		return *this;
	}

private:
	/* The actual kwarg value */
	tValueType m_value;

	template<typename... Args>
	explicit cKwArg(Args&&... args): m_value(std::forward<Args>(args)...) {}

	tValueType& get() noexcept { return m_value; }
	const tValueType& get() const noexcept { return m_value; }
	tValueType move() { return std::move(m_value); }

	template<iKwArg...> friend class cKwPack;
	template<int key> friend tKwType<key>& KwGet(cKwPack<>&, const tKwType<key>&);
	template<int key> friend tKwType<key>& KwGet(cKwPack<>&, tKwType<key>&&);
};
/* ========== Access functions for empty packs ====================================================================== */
template<int key>
inline tKwType<key>& KwGet(cKwPack<>&, const tKwType<key>& default_) { return (cKwArg<key>::Instance = default_).get(); }
template<int key>
inline tKwType<key>& KwGet(cKwPack<>&, tKwType<key>&& default_ = {}) { return (cKwArg<key>::Instance = std::forward<tKwType<key>>(default_)).get(); }
template<int key>
inline const tKwType<key>& KwGet(const cKwPack<>&, const tKwType<key>& default_) { return default_; }
template<int key>
inline const tKwType<key>& KwGet(const cKwPack<>& p, tKwType<key>&& default_ = {}) { return KwGet<key>(const_cast<cKwPack<>&>(p), std::forward<tKwType<key>>(default_)); }//(cKwArg<key>::Instance = std::forward<tKwType<key>>(default_)).get(); }
template<int key>
inline tKwType<key> KwMove(cKwPack<>&, const tKwType<key>& default_) { return default_; }
template<int key>
inline tKwType<key> KwMove(cKwPack<>&, tKwType<key>&& default_ = {}) { return default_; }
/* ========== Access functions for non empty packs ================================================================== */
template<int key, iKwArg First, iKwArg... Rest, typename... Default>
tKwType<key>& KwGet(cKwPack<First, Rest...>&, Default&&...);
template<int key, iKwArg First, iKwArg... Rest, typename... Default>
tKwType<key> KwMove(cKwPack<First, Rest...>&, Default&&...);
/* ========== Default empty kwarg pack ============================================================================== */
template<iKwArg...>
class cKwPack final {
public:
	cKwPack() = default;
	cKwPack(const cKwPack&) = delete;
	cKwPack& operator=(const cKwPack&) = delete;
};
/* ========== Non-empty kwarg pack specialization =================================================================== */
template<iKwArg First, iKwArg... Rest>
class cKwPack<First, Rest...> final {
public:
	explicit cKwPack(First& f, Rest&... k) : m_first(f), m_rest(k...) {}

private:
	First&           m_first; // The first kwarg of the pack
	cKwPack<Rest...> m_rest;  // The rest of the kwargs

	template<int key, typename... Default>
	friend tKwType<key>& KwGet(cKwPack& pack, Default&&... default_) {
		if constexpr (key == std::remove_cvref_t<First>::Key)
			return pack.m_first.get();
		else
			return KwGet<key>(pack.m_rest, std::forward<Default>(default_)...);
	}
	template<int key, typename... Default>
	friend tKwType<key> KwMove(cKwPack& pack, Default&&... default_) {
		if constexpr (key == std::remove_cvref_t<First>::Key)
			return pack.m_first.move();
		else
			return KwMove<key>(pack.m_rest, std::forward<Default>(default_)...);
	}
};
/* ========== Final touches to support all kinds of references ====================================================== */
template<int key, iKwArg... KwArgs, typename... Default>
inline const tKwType<key>& KwGet(const cKwPack<KwArgs...>&& pack, Default&&... default_) { return KwGet<key>(const_cast<cKwPack<KwArgs...>&>(pack));}
template<int key, iKwArg... Kwargs, typename... Default>
inline tKwType<key> KwMove(cKwPack<Kwargs...>&& pack, Default&&... default_) { return KwMove<key>(pack, std::forward<Default>(default_)...); }
/*
 * KWARG_DECLARE():
 * - First specialize kwtraits so that tKwType<> works
 * - Then initialize the static instance of the kwarg
 * - Finally, create a reference with the specified name
 */
#define KW_DECLARE(name, key, type, ...)\
template<> struct detail::kwarg::kwtraits<key> { using value_type = type; };\
template<> inline cKwArg<key> cKwArg<key>::Instance{ __VA_ARGS__ };\
inline auto& name = cKwArg<key>::Instance;
#endif //GREEKBOT_KWARG_H