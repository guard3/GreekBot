#ifndef _GREEKBOT_CHANNEL_H_
#define _GREEKBOT_CHANNEL_H_
#include "Common.h"

enum eChannelType {
	CHANNEL_GUILD_TEXT,
	CHANNEL_DM,
	CHANNEL_GUILD_VOICE,
	CHANNEL_GROUP_DM,
	CHANNEL_CATEGORY,
	CHANNEL_GUILD_NEWS,
	CHANNEL_GUILD_STORE,
	GUILD_NEWS_THREAD = 10,
	GUILD_PUBLIC_THREAD,
	GUILD_PRIVATE_THREAD,
	GUILD_STAGE_VOICE
};

class cChannel final {
private:
	cSnowflake id;
	eChannelType type;
	// ...
	char* name;

public:
	explicit cChannel(const json::value& v) : id(v.at("id").as_string().c_str()), type((eChannelType)v.at("type").as_int64()) {
		if (auto c = v.if_object()->if_contains("name")) {
			if (auto s = c->if_string()) {
				name = new char[s->size() + 1];
				strcpy(name, s->c_str());
				return;
			}
		}
		name = nullptr;
	}
	cChannel(const cChannel& o) : id(o.id), type(o.type) {
		if (o.name) {
			name = new char[strlen(o.name) + 1];
			strcpy(name, o.name);
		}
		else name = nullptr;
	}
	cChannel(cChannel&& o) : id(o.id), type(o.type), name(o.name) { o.name = nullptr; }
	~cChannel() { delete[] name; }

	cChannel& operator=(cChannel o) {
		id = o.id;
		type = o.type;
		auto t = name;
		name = o.name;
		o.name = t;
		return *this;
	}

	chSnowflake GetId() const { return &id; }
	eChannelType GetType() const { return type; }
	const char* GetName() const { return name; }
};
typedef   hHandle<cChannel>   hChannel;
typedef  chHandle<cChannel>  chChannel;
typedef  uhHandle<cChannel>  uhChannel;
typedef uchHandle<cChannel> uchChannel;
typedef  shHandle<cChannel>  shChannel;
typedef schHandle<cChannel> schChannel;
#endif /* _GREEKBOT_CHANNEL_H_ */
