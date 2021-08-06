#pragma once
#ifndef _GREEKBOT_TYPES_H_
#define _GREEKBOT_TYPES_H_
#include <memory>

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
	char     m_str[24]; // The snowflake as a string
	uint64_t m_int;     // The snowflake as a 64-bit integer
	
public:
	cSnowflake(const char* str) {
		strcpy(m_str, str);
		char* temp;
		m_int = strtoull(str, &temp, 10);
	}
	
	/* Comparison operators */
	bool operator==(const cSnowflake& other) { return m_int == other.m_int;  }
	bool operator==(uint64_t i)              { return m_int == i;            }
	bool operator==(const char* s)           { return 0 == strcmp(m_str, s); }
	
	/* Attributes */
	const char *ToString() const { return m_str; }
	uint64_t    ToInt()    const { return m_int; }
	
	/* Snowflake components - https://discord.com/developers/docs/reference#snowflakes */
	time_t GetTimestamp()         const { return static_cast<time_t>((m_int >> 22) / 1000 + 1420070400); }
	int    GetInternalWorkerID()  const { return static_cast<int   >((m_int >> 17) & 0x1F             ); }
	int    GetInternalProcessID() const { return static_cast<int   >((m_int >> 12) & 0x1F             ); }
	int    GetIncrement()         const { return static_cast<int   >( m_int        & 0xFFF            ); }
};
typedef   hHandle<cSnowflake>   hSnowflake; // handle
typedef  chHandle<cSnowflake>  chSnowflake; // const handle
typedef  uhHandle<cSnowflake>  uhSnowflake; // unique handle
typedef uchHandle<cSnowflake> uchSnowflake; // unique const handle
typedef  shHandle<cSnowflake>  shSnowflake; // shared handle
typedef schHandle<cSnowflake> schSnowflake; // shared const handle

inline bool operator==(uint64_t i, const cSnowflake& snowflake) { return i == snowflake.ToInt(); }
inline bool operator==(const char* s, const cSnowflake& snowflake) { return 0 == strcmp(s, snowflake.ToString()); }

#endif /* _GREEKBOT_TYPES_H_ */
