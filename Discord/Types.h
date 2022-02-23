#pragma once
#ifndef _GREEKBOT_TYPES_H_
#define _GREEKBOT_TYPES_H_
#include "json.h"
#include <cinttypes>

/* ========== Handle types ========== */
template<typename T> // handle
using   hHandle = T*;
template<typename T> // const handle
using  chHandle = const T*; // ch: const handle
template<typename T> // unique handle
using  uhHandle = std::unique_ptr<T>;//std::remove_const_t<T>>;
template<typename T> // unique const handle
using uchHandle = std::unique_ptr<const T>;
template<typename T> // shared handle
using  shHandle = std::shared_ptr<T>;
template<typename T> // shared const handle
using schHandle = std::shared_ptr<const T>;

/* ========== Handle creation functions ========== */
class cHandle final {
private:
	cHandle() = default;

	template<typename T> struct is_handle              : std::false_type {};
	template<typename T> struct is_handle<T*>          : std::true_type  {};
	template<typename T> struct is_handle<uhHandle<T>> : std::true_type  {};
	template<typename T> struct is_handle<shHandle<T>> : std::true_type  {};

	template<typename T> struct remove_h { typedef T Type; };
	template<typename T> struct remove_h<hHandle<T>> { typedef T Type; };
	template<typename T> struct remove_h<uhHandle<T>> { typedef T Type; };
	template<typename T> struct remove_h<shHandle<T>> { typedef T Type; };

public:
	template<typename T, typename... Args>
	static uhHandle<T> MakeUnique(Args&&... args) { return std::make_unique<T>(std::forward<Args>(args)...); }
	template<typename T, typename... Args>
	static shHandle<T> MakeShared(Args&&... args) { return std::make_shared<T>(std::forward<Args>(args)...); }
	template<typename T, typename... Args>
	static uchHandle<T> MakeUniqueConst(Args&&... args) { return MakeUnique<T>(std::forward<Args>(args)...); }
	template<typename T, typename... Args>
	static schHandle<T> MakeSharedConst(Args&&... args) { return MakeShared<T>(std::forward<Args>(args)...); }

	template<typename T, typename... Args>
	static uhHandle<T> MakeUniqueNoEx(Args&&... args) {
		try {
			return MakeUnique<T>(std::forward<Args>(args)...);
		}
		catch (...) {
			return uhHandle<T>();
		}
	}
	template<typename T, typename... Args>
	static shHandle<T> MakeSharedNoEx(Args&&... args) {
		try {
			return MakeShared<T>(std::forward<Args>(args)...);
		}
		catch (...) {
			return shHandle<T>();
		}
	}
	template<typename T, typename... Args>
	static uchHandle<T> MakeUniqueConstNoEx(Args&&... args) { return MakeUniqueNoEx<T>(std::forward<Args>(args)...); }
	template<typename T, typename... Args>
	static schHandle<T> MakeSharedConstNoEx(Args&&... args) { return MakeSharedNoEx<T>(std::forward<Args>(args)...); }

	template<typename T>
	static inline constexpr bool IsHandleType = is_handle<T>::value;
	template<typename T>
	using RemoveHandle = typename remove_h<std::remove_cv_t<T>>::Type;
	template<typename T>
	using RemoveHandleC = std::remove_const_t<RemoveHandle<T>>;
	template<typename T>
	using RemoveHandleV = std::remove_volatile_t<RemoveHandle<T>>;
	template<typename T>
	using RemoveHandleCV = std::remove_cv_t<RemoveHandle<T>>;
};

/* ========== Discord snowflake ========== */
class cSnowflake final {
private:
	char     m_str[24]; // The snowflake as a string
	uint64_t m_int;     // The snowflake as a 64-bit integer
	
public:
	cSnowflake() noexcept : m_int(0), m_str("0") {}
	cSnowflake(uint64_t i) noexcept : m_int(i) { sprintf(m_str, "%" PRIu64, i); }
	cSnowflake(const char* str) noexcept : m_int(strtoull(str, nullptr, 10)) { strcpy(m_str, str); }
	cSnowflake(const json::value& v) : cSnowflake(v.as_string().c_str()) {}

	bool operator==(const cSnowflake& o) const { return m_int == o.m_int; }
	bool operator!=(const cSnowflake& o) const { return m_int != o.m_int; }
	bool operator< (const cSnowflake& o) const { return m_int <  o.m_int; }
	bool operator> (const cSnowflake& o) const { return m_int >  o.m_int; }
	bool operator<=(const cSnowflake& o) const { return m_int <= o.m_int; }
	bool operator>=(const cSnowflake& o) const { return m_int >= o.m_int; }

	/* Attributes */
	const char *ToString() const { return m_str; }
	uint64_t    ToInt()    const { return m_int; }
	
	/* Snowflake components - https://discord.com/developers/docs/reference#snowflakes */
	time_t GetTimestamp()         const { return (m_int >> 22) / 1000 + 1420070400; }
	int    GetInternalWorkerID()  const { return (m_int >> 17) & 0x1F;              }
	int    GetInternalProcessID() const { return (m_int >> 12) & 0x1F;              }
	int    GetIncrement()         const { return  m_int        & 0xFFF;             }
};
typedef   hHandle<cSnowflake>   hSnowflake; // handle
typedef  chHandle<cSnowflake>  chSnowflake; // const handle
typedef  uhHandle<cSnowflake>  uhSnowflake; // unique handle
typedef uchHandle<cSnowflake> uchSnowflake; // unique const handle
typedef  shHandle<cSnowflake>  shSnowflake; // shared handle
typedef schHandle<cSnowflake> schSnowflake; // shared const handle

/* ========== Color ========== */
class cColor final {
private:
	int32_t m_value;

public:
	static inline constexpr int32_t NO_COLOR = 0;
	cColor() : m_value(NO_COLOR) {}
	cColor(int32_t v) : m_value(v) {}
	cColor(const json::value& v) : cColor(v.as_int64()) {}

	uint8_t GetRed()   const { return (m_value >> 16) & 0xFF; }
	uint8_t GetGreen() const { return (m_value >>  8) & 0xFF; }
	uint8_t GetBlue()  const { return  m_value        & 0xFF; }

	int ToInt() const { return m_value; }
	operator int() const { return m_value; }
	operator bool() const { return m_value != NO_COLOR; }
	bool operator!() const { return m_value == NO_COLOR; }
};

#endif /* _GREEKBOT_TYPES_H_ */
