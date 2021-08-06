#pragma once
#ifndef _GREEKBOT_NET_H_
#define _GREEKBOT_NET_H_
#include <string>

class cNet final {
private:
	cNet() {}
	
public:
	static bool GetHttpsRequest(const char* host, const char* path, const char* auth, std::string& response);
};

#endif /* _GREEKBOT_NET_H_ */
