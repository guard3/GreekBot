#pragma once
#ifndef _GREEKBOT_UTILS_H_
#define _GREEKBOT_UTILS_H_
#include <string>

/* A helper class with various useful functions */
class cUtils final {
private:
	cUtils() = default;

public:
	/* Random functions */
	static float Random();
	static int Random(int a, int b);
	
	/* Logger functions */
	static void PrintErr(const char* fmt, ...);
	static void PrintLog(const char* fmt, ...);

	/* C style formatting for std::string */
	static std::string Format(const char* fmt, ...);
	
	static const char* GetOS() {
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
};

#endif /* _GREEKBOT_UTILS_H_ */