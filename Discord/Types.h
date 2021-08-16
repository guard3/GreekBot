#pragma once
#ifndef _GREEKBOT_TYPES_H_
#define _GREEKBOT_TYPES_H_
#include "json.h"
#include <cinttypes>

/* ========== Handle types ========== */
template<typename T> // handle
using   hHandle = std::remove_const_t<T>*;
template<typename T> // const handle
using  chHandle = const T*; // ch: const handle
template<typename T> // unique handle
using  uhHandle = std::unique_ptr<std::remove_const_t<T>>;
template<typename T> // unique const handle
using uchHandle = std::unique_ptr<const T>;
template<typename T> // shared handle
using  shHandle = std::shared_ptr<std::remove_const_t<T>>;
template<typename T> // shared const handle
using schHandle = std::shared_ptr<const T>;

/* ========== Handle creation functions ========== */
class cHandle final {
private:
	cHandle() = default;

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
};

/* ========== Discord snowflake ========== */
class cSnowflake final {
private:
	char     m_str[24]{}; // The snowflake as a string
	uint64_t m_int;     // The snowflake as a 64-bit integer
	
public:
	typedef uint64_t tSnowflake;

	cSnowflake(tSnowflake i) noexcept : m_int(i) { sprintf(m_str, "%" PRIu64, i); }
	cSnowflake(const char* str) noexcept {
		strcpy(m_str, str);
		char* temp;
		m_int = strtoull(str, &temp, 10);
	}
	explicit cSnowflake(const json::value& v) : cSnowflake(v.as_string().c_str()) {}
	
	/* Attributes */
	[[nodiscard]] const char *ToString() const { return m_str; }
	[[nodiscard]] uint64_t    ToInt()    const { return m_int; }
	
	/* Snowflake components - https://discord.com/developers/docs/reference#snowflakes */
	[[nodiscard]] time_t GetTimestamp()         const { return static_cast<time_t>((m_int >> 22) / 1000 + 1420070400); }
	[[nodiscard]] int    GetInternalWorkerID()  const { return static_cast<int   >((m_int >> 17) & 0x1F             ); }
	[[nodiscard]] int    GetInternalProcessID() const { return static_cast<int   >((m_int >> 12) & 0x1F             ); }
	[[nodiscard]] int    GetIncrement()         const { return static_cast<int   >( m_int        & 0xFFF            ); }
};
typedef   hHandle<cSnowflake>   hSnowflake; // handle
typedef  chHandle<cSnowflake>  chSnowflake; // const handle
typedef  uhHandle<cSnowflake>  uhSnowflake; // unique handle
typedef uchHandle<cSnowflake> uchSnowflake; // unique const handle
typedef  shHandle<cSnowflake>  shSnowflake; // shared handle
typedef schHandle<cSnowflake> schSnowflake; // shared const handle

#endif /* _GREEKBOT_TYPES_H_ */
