#pragma once
#ifndef _GREEKBOT_NET_H_
#define _GREEKBOT_NET_H_
#include "json.h"

class cNet final {
private:
	cNet() = default;
	
public:
	static unsigned int GetHttpsRequest(const char* host, const char* path, const char* auth, std::string& response);
	static bool PostHttpsRequest(const char* host, const char* path, const char* auth, const json::object& obj);
	static bool PostHttpsRequest(const char* host, const char* path, const char* auth, json::object&& obj);
	static bool PutHttpsRequest(const char* host, const char* path, const char* auth);
	static bool DeleteHttpsRequest(const char* host, const char* path, const char* auth);
	static bool PatchHttpsRequest(const char* host, const char* path, const char* auth, const std::string& data);
	static bool PatchHttpsRequest(const char* host, const char* path, const char* auth, const json::object& obj);
};

#endif /* _GREEKBOT_NET_H_ */
