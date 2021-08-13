#pragma once
#ifndef _GREEKBOT_TYPES_H_
#define _GREEKBOT_TYPES_H_
#include "json.h"
#include <cinttypes>

/* === Handle types === */

/* Type hHandle; h: handle */
template<typename T>
using hHandle = typename std::remove_const<T>::type*;

/* Type chHandle; ch: const handle */
template<typename T>
using chHandle = const T*; // ch: const handle

/* Type uhHandle; uh: unique handle */
template<typename T>
using uhHandle = std::unique_ptr<typename std::remove_const<T>::type>;

/* Type uchHandle; uch: unique const handle */
template<typename T>
using uchHandle = std::unique_ptr<const T>;

/* Type shHandle; sh: shared handle */
template<typename T>
using shHandle = std::shared_ptr<typename std::remove_const<T>::type>;

/* Type schHandle; sch: shared const handle */
template<typename T>
using schHandle = std::shared_ptr<const T>;

/* === Discord snowflake === */
class cSnowflake final {
private:
	char     m_str[24]{}; // The snowflake as a string
	uint64_t m_int;     // The snowflake as a 64-bit integer
	
public:
	cSnowflake(uint64_t i) : m_int(i) { sprintf(m_str, "%" PRIu64, i); }
	cSnowflake(const char* str) {
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
