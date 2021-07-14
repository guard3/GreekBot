#pragma once
#ifndef _GREEKBOT_DISCORD_H_
#define _GREEKBOT_DISCORD_H_

#include <boost/json.hpp>
namespace json = boost::json;

/* A helper class that performs HTTP requests to the API endpoint */
class cDiscord final {
private:
	cDiscord() {}
	
public:
	static bool GetGateway(const char* auth, json::value& value);
	
};

#endif /* _GREEKBOT_DISCORD_H_*/
