#ifndef DISCORD_FLAGSET_H
#define DISCORD_FLAGSET_H
#include <concepts>

/**
 * A helper class to be used as a base for custom enum-class-like types that model BitMaskType.
 * It is a simple wrapper around UnderlyingType
 *
 * Functions for testing set flags are provided, similar to what is offered by std::bitset
 */
template<std::unsigned_integral UnderlyingType>
class cFlagSet {
	UnderlyingType value;

public:
	constexpr cFlagSet(UnderlyingType v) noexcept : value(v) {}

	template<typename T>
	constexpr bool operator==(this T lhs, T rhs) noexcept { return lhs.value == rhs.value; }

	template<typename T>
	constexpr T operator~(this T rhs) noexcept { return T(~rhs.value); }

	template<typename T>
	constexpr T operator&(this T lhs, T rhs) noexcept { return T(lhs.value & rhs.value); }
	template<typename T>
	constexpr T operator|(this T lhs, T rhs) noexcept { return T(lhs.value | rhs.value); }
	template<typename T>
	constexpr T operator^(this T lhs, T rhs) noexcept { return T(lhs.value ^ rhs.value); }

	template<typename T>
	constexpr T& operator&=(this T& lhs, T rhs) noexcept { return lhs = lhs & rhs; }
	template<typename T>
	constexpr T& operator|=(this T& lhs, T rhs) noexcept { return lhs = lhs | rhs; }
	template<typename T>
	constexpr T& operator^=(this T& lhs, T rhs) noexcept { return lhs = lhs ^ rhs; }

	/**
	 * Checks if any of the bits are set
	 * @return \c true if any bits are set, otherwise \c false
	 */
	template<typename T>
	constexpr bool Any(this T self) noexcept { return self.value; }

	/**
	 * Checks if none of the bits are set
	 * @return \c true if none of the bits are set, otherwise \c false
	 */
	template<typename T>
	constexpr bool None(this T self) noexcept { return !self.value; }

	/**
	 * Tests whether all the bits specified by \p flags are set
	 * @param flags The flags to test against
	 * @return \c true if all the bits specified by \p flags are set, otherwise \c false
	 */
	template<typename T>
	constexpr bool TestAll(this T self, T flags) noexcept { return (self & flags) == flags; }

	/**
	 * Tests whether any of the bits specified by \p flags are set
	 * @param flags The flags to test against
	 * @return \c true if any of the bits specified by \p flags are set, otherwise \c false
	 */
	template<typename T>
	constexpr bool TestAny(this T self, T flags) noexcept { return (self & flags).Any(); }

	/**
	 * Tests whether none of the bits specified by \p flags are set
	 * @param flags The flags to test against
	 * @return \c true if none of the bits specified by \p flags are set, otherwise \c false
	 */
	template<typename T>
	constexpr bool TestNone(this T self, T flags) noexcept { return (self & flags).None(); }
};
#endif //DISCORD_FLAGSET_H