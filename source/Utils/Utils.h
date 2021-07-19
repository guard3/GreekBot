#pragma once
#ifndef _GREEKBOT_UTILS_H_
#define _GREEKBOT_UTILS_H_
#include <string>

/* A helper class with various useful functions */
class cUtils final {
private:
	cUtils() {}
	
	
	static void Print(FILE* f, const char* comment, const char* fmt, va_list args);
	
public:
	/* Logger functions */
	static void PrintErr(const char* fmt, ...);
	static void PrintLog(const char* fmt, ...);
	
	static std::string GetHttpsRequest(const char* host, const char* path = "/", const char* auth = nullptr);
};

#endif /* _GREEKBOT_UTILS_H_ */
