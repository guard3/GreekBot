#include "Common.h"
#include "json.h"
#include <algorithm>
#include <fmt/format.h>

template<int radix, std::unsigned_integral I>
static char* unsigned_integral_to_string(I n, char* buffer) {
	/* Save the input buffer pointer */
	char* result = buffer;
	/* Write string from left to right */
	do {
		if constexpr (radix == 8) {
			*buffer++ = '0' + (n & 7);
			n >>= 3;
		}
		else if constexpr (radix == 10) {
			*buffer++ = '0' + (n % 10);
			n /= 10;
		}
		else {
			static const char hex[] = "0123456789abcdef";
			*buffer++ = hex[n & 15];
			n >>= 4;
		}
	} while (n);
	/* Add the null terminator */
	*buffer = '\0';
	/* Reverse the result */
	std::reverse(result, buffer);
	return result;
}

template<std::integral I>
static char* itoa_(I n, char* buffer, int radix) {
	switch (radix) {
		case 8:
			return unsigned_integral_to_string<8>(n, buffer);
		case 10:
			return unsigned_integral_to_string<10>(n, buffer);
		case 16:
			return unsigned_integral_to_string<16>(n, buffer);
		default:
			errno = EINVAL;
			return nullptr;
	}
}

char* __attribute__((weak)) utoa(unsigned n, char* buffer, int radix) { return itoa_(n, buffer, radix); }
char* __attribute__((weak)) ultoa(unsigned long n, char* buffer, int radix) { return itoa_(n, buffer, radix); }
char* __attribute__((weak)) ulltoa(unsigned long long n, char* buffer, int radix) { return itoa_(n, buffer, radix); }

cSnowflake
tag_invoke(json::value_to_tag<cSnowflake>, const json::value& v) {
	return json::value_to<std::string_view>(v).data();
}
cColor
tag_invoke(json::value_to_tag<cColor>, const json::value& v) {
	return v.to_number<int32_t>();
}

fmt::format_context::iterator fmt::formatter<cSnowflake>::format(const cSnowflake& snowflake, format_context& ctx) const {
	return formatter<string_view>::format(snowflake.ToString(), ctx);
}