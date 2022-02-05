#include "Utils.h"
#include <cstdarg>
#include <random>

/* Random engine stuff */
static std::random_device s_rd;
static std::mt19937 s_gen(s_rd());
static std::uniform_real_distribution<float> s_dis(0.0f, 0.1f);

float cUtils::Random() {
	return s_dis(s_gen);
}

int cUtils::Random(int a, int b) {
	std::uniform_int_distribution<int> dis(a, b);
	return dis(s_gen);
}

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
	va_list args1;
	va_start(args1, fmt);
	std::string str(10, '\0');
	int str_len = vsnprintf(str.data(), str.size() + 1, fmt, args1);
	va_end(args1);
	if (str_len >= 0) {
		str.resize(str_len);
		if (str_len > 10) {
			va_list args2;
			va_start(args2, fmt);
			vsprintf(str.data(), fmt, args2);
			va_end(args2);
		}
	}
	return str;
}