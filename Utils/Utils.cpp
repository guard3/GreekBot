#include "Utils.h"
#include <cstdarg>
#include <random>

/* Random engine stuff */
static std::mt19937 s_gen(std::random_device{}());
static std::uniform_real_distribution<float> s_rDis;
static std::uniform_int_distribution<int> s_iDis;

float cUtils::Random() { return s_rDis(s_gen, decltype(s_rDis)::param_type(0.0f, 1.0f)); }

int cUtils::Random(int a, int b) { return s_iDis(s_gen, decltype(s_iDis)::param_type(a, b)); }

static void print(FILE* f, const char* comment, const char* fmt, va_list args) {
	fputs(comment, f);
	vfprintf(f, fmt, args);
	fputc('\n', f);
}

void cUtils::PrintErr(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	print(stderr, "[ERR] ", fmt, args);
	va_end(args);
}

void cUtils::PrintLog(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	print(stdout, "[LOG] ", fmt, args);
	va_end(args);
}

std::string
cUtils::Format(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	std::string str(255, '\0');
	int str_len = vsnprintf(str.data(), str.size() + 1, fmt, args);
	va_end(args);
	if (str_len < 0)
		return {};
	str.resize(str_len);
	if (str_len > 255) {
		va_start(args, fmt);
		vsprintf(str.data(), fmt, args);
		va_end(args);
	}
	return str;
}