#pragma once
#ifndef _GREEKBOT_NET_H_
#define _GREEKBOT_NET_H_
#include "json.h"

class cNet final {
private:
	cNet() = default;
	
public:
	static int    GetHttpsRequest(const char* host, const char* path, const char* auth, std::string& response);
	static int   PostHttpsRequest(const char* host, const char* path, const char* auth, const json::object& obj);
	static int  PatchHttpsRequest(const char* host, const char* path, const char* auth, const json::object& obj);
	static int    PutHttpsRequest(const char* host, const char* path, const char* auth);
	static int DeleteHttpsRequest(const char* host, const char* path, const char* auth);
};

#endif /* _GREEKBOT_NET_H_ */
