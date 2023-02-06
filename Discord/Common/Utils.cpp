#include "Utils.h"
#include "date_time.h"

/* Random engine stuff */
static std::random_device g_rd;
std::mt19937    cUtils::ms_gen(g_rd());
std::mt19937_64 cUtils::ms_gen64(g_rd());

void
cUtils::print(FILE* f, const char* comment, char nl, const char* fmt, ...) {
	fputs(comment, f);
	va_list args;
	va_start(args, fmt);
	vfprintf(f, fmt, args);
	va_end(args);
	fputc(nl, f);
}

std::string
cUtils::format(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	char temp[1024];
	int str_len = vsnprintf(temp, 1024, fmt, args);
	va_end(args);
	if (str_len < 0)
		return {};
	if (str_len < 1024)
		return temp;
	std::string str(str_len, '\0');
	va_start(args, fmt);
	vsprintf(str.data(), fmt, args);
	va_end(args);
	return str;
}

std::chrono::sys_seconds
cUtils::ParseTimestamp(const std::string& t) {
	return std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::from_time_t(posix_time::to_time_t(posix_time::from_iso_extended_string(t.substr(0, t.find_last_of("+-Z"))))));
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