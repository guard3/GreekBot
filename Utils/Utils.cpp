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
