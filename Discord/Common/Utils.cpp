#include "Utils.h"
#include "date_time.h"

/* Random engine stuff */
static std::random_device g_rd;
std::mt19937    cUtils::ms_gen(g_rd());
std::mt19937_64 cUtils::ms_gen64(g_rd());

std::chrono::sys_seconds
cUtils::ParseTimestamp(std::string_view t) {
	// Dirty hack since, for some reason, DateTime doesn't recognize timezones...
	std::string_view sub = t.substr(0, t.find_last_of("+-Z"));
	return std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::from_time_t(posix_time::to_time_t(posix_time::from_iso_extended_string(std::string(sub.begin(), sub.end())))));
}

std::string
cUtils::PercentEncode(std::string_view sv) {
	/* A lookup table to check if a character is unreserved or not */
	static const bool unreserved_char[256] {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
		0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1,
		0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};
	/* Reserve at most 3 times as many characters for the output string */
	std::string result;
	result.reserve(sv.size() * 3);
	/* Parse the input */
	for (unsigned char c : sv) {
		if (unreserved_char[c])
			result.push_back(static_cast<char>(c));
		else {
			static const char hex[] = "0123456789ABCDEF";
			result.push_back('%');
			result.push_back(hex[c >> 4]);
			result.push_back(hex[c & 15]);
		}
	}
	/* Return result */
	result.shrink_to_fit();
	return result;
}

const char*
cUtils::GetOS() {
#ifdef _WIN32
	return "Windows";
#elif defined __APPLE__ || defined __MACH__
	return "macOS";
#elif defined __linux__
	return "Linux";
#elif defined __FreeBSD__
	return "FreeBSD";
#else
	return "Unix";
#endif
}