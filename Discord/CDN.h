#ifndef GREEKBOT_CDN_H
#define GREEKBOT_CDN_H
#include "User.h"

class cCDN final {
public:
	cCDN() = delete;

	static std::string GetDefaultUserAvatar(crefUser user);
	static std::string GetUserAvatar(crefUser user);
};
#endif /* GREEKBOT_CDN_H */